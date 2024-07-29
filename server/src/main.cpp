#include <nlohmann/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <websocket/server_ws.hpp>

#include <optional>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include <sstream>
#include <cstdlib>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "api/SCAPI.hpp"
#include "messages.hpp"
#include "version.hpp"

class Config {
  public:
	Config(const std::filesystem::path& path) { fromJson(path); }

	std::chrono::system_clock::duration sc_api_poll_rate;
	std::string sc_api_secret;
	std::string sc_api_id;

	void fromJson(const std::filesystem::path& path) {
		auto config = nlohmann::json::parse(std::ifstream(path));
		this->sc_api_poll_rate = std::chrono::seconds(config["sc_api_poll_rate"]);
		this->sc_api_id = config["sc_api_id"];
		this->sc_api_secret = config["sc_api_secret"];
	}
};

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

std::unordered_set<std::string> auth;

constexpr int port = 8001;

namespace persistent {
std::unordered_map<size_t, msg::Alert> alerts;
std::unordered_map<size_t, std::unordered_set<size_t>> tabs;
}  // namespace persistent

[[nodiscard]] std::filesystem::path getExecutablePath() noexcept {
	std::error_code ec;
	auto path = std::filesystem::read_symlink("/proc/self/exe", ec);
	if (ec)
		return "";
	return path.parent_path();
}

[[nodiscard]] std::unordered_map<std::string, std::string> parseDatabase(const std::filesystem::path& path) {
	std::unordered_map<std::string, std::string> map;

	if (std::filesystem::is_directory(path)) {
		for (const auto& entry: std::filesystem::directory_iterator(path)) {
			if (entry.path().string().find("_variants") != std::string::npos)
				continue;

			auto subMap = parseDatabase(entry.path());
			map.insert(subMap.begin(), subMap.end());
		}
	} else if (path.extension() == ".json") {
		auto j = nlohmann::json::parse(std::ifstream(path));

		std::string id = path.stem().string();
		std::string name = j["name"]["lines"]["ru"];
		map[id] = name;
	}

	return map;
}

void updateDB(const std::filesystem::path db_path) {
	if (!std::filesystem::exists(db_path)) {
		std::string command = "git clone https://github.com/EXBO-Studio/stalcraft-database/ " + db_path.string();
		int result = std::system(command.c_str());
		if (result != 0)
			throw std::runtime_error("Failed to clone the repository. Error code: " + std::to_string(result));
	} else {
		std::string command = "cd " + db_path.string() + " && git pull";
		int result = std::system(command.c_str());
		if (result != 0)
			throw std::runtime_error("Failed to update the repository. Error code: " + std::to_string(result));
	}
}

[[nodiscard]] std::filesystem::path getItemDB() {
	const std::filesystem::path db_path = getExecutablePath() / "db";
	updateDB(db_path);
	return db_path;
}

void sendResponse(msg::Response&& response, std::shared_ptr<WsServer::Connection>& connection) {
	std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(response);
	}
	std::string str = ss.str();
	connection->send(str, [](const SimpleWeb::error_code& ec) {
		if (ec) {
			std::cout << "Server: Error sending message. " <<
			  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code
			  // meanings
			  "Error: " << ec << ", error message: " << ec.message() << std::endl;
		}
	});
}

// TODO: Update somewhere else, not when user adds alerts
std::unordered_map<std::string, std::string> getItems() {
	static const std::chrono::hours period(1);
	static std::chrono::system_clock::time_point last_update;
	static std::unordered_map<std::string, std::string> items;
	const auto now = std::chrono::system_clock::now();

	if (items.empty() || now - last_update >= period) {
		items = parseDatabase(getItemDB() / "ru" / "items");
		last_update = now;
		std::printf("loaded: %lu items\n", items.size());
	}

	return items;
}

void processRequest(msg::Request&& request, std::shared_ptr<WsServer::Connection>& connection) {
	bool ip_match = auth.contains(connection->remote_endpoint().address().to_string());
	if (!ip_match && !std::holds_alternative<msg::RequestPassword>(request.request))
		return;
	std::visit(
	  overloaded{
		[&](msg::RequestPing& ping) {
			printf("Recieved ping: %s\n", ping.str.c_str());
			sendResponse({msg::ResponsePing{.str = ping.str}}, connection);
		},
		[&](msg::Version& ver) {
			printf("Client connected, version %d %s\n", ver.build_number, ver.version.c_str());
			sendResponse(
			  {msg::Version{.build_number = BUILD_NUMBER, .version = BUILD_VERSION, .msg_hash = MSG_HASH}},
			  connection
			);
		},
		[&](msg::RequestItems& r) {
			sendResponse(
			  {msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts, .tabs = persistent::tabs}},
			  connection
			);
		},
		[&](msg::RequestHistory& r) {

		},
		[&](msg::RequestRemoveAlert& alert) {
			if (persistent::alerts.contains(alert.id))
				persistent::alerts.erase(alert.id);

			for (auto& [tab_id, alert_set]: persistent::tabs)
				if (alert_set.contains(alert.id))
					alert_set.erase(alert.id);

			sendResponse(
			  {msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts, .tabs = persistent::tabs}},
			  connection
			);
		},
		[&](msg::RequestSwitchAlert& alert) {
			if (persistent::alerts.contains(alert.id))
				persistent::alerts[alert.id].enabled = alert.state;
			sendResponse(
			  {msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts, .tabs = persistent::tabs}},
			  connection
			);
		},

		[&](msg::RequestAddAlert& data) {
			if (!data.alert.item.empty()) {
				for (auto& [tab_id, tab_alert_ids]: persistent::tabs) {
					if (tab_id != data.tab_id && tab_alert_ids.find(data.alert_id) != tab_alert_ids.end()) {
						tab_alert_ids.erase(data.alert_id);
						break;
					}
				}
				persistent::tabs[data.tab_id].emplace(data.alert_id);
				persistent::alerts[data.alert_id] = data.alert;
			} else {
				persistent::tabs[data.tab_id];
			}

			sendResponse(
			  {msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts, .tabs = persistent::tabs}},
			  connection
			);
		},
		[&](msg::RequestPassword& pass) {
			if (pass.password == "1131") {
				auth.insert(connection->remote_endpoint().address().to_string());
				printf("auth: %s\n", connection->remote_endpoint().address().to_string().c_str());
				sendResponse(msg::Response{msg::ResponsePassword{true}}, connection);
			}
		}
	  },
	  request.request
	);
}

void parseLots(nlohmann::json& json, std::vector<nlohmann::json>& result) {
	auto lots = json["lots"];
	size_t count = 0;
	for (auto& [_, lot]: lots.items()) {
		result.push_back(lot);
		count++;
	}
	printf("Received %lu\n", count);
}

std::vector<nlohmann::json> loadAll(SCAPI& scapi, const std::string& id) {
	printf("Requested all: %s\n", id.c_str());

	std::vector<nlohmann::json> result;
	size_t limit = 200;
	size_t offset = 0;

	auto fetchAndParse = [&](size_t currentOffset) {
		auto active = scapi.getActiveLots(
		  {.region_id = "RU", .item_id = id, .limit = limit, .offset = currentOffset, .additional = true},
		  Sort::Criterion::BuyoutPrice,
		  Sort::Order::Ascending
		);
		parseLots(active, result);
		return active["total"].get<int>();
	};

	int total = fetchAndParse(offset);
	offset += limit;
	total -= limit;

	while (total > 0) {
		fetchAndParse(offset);
		offset += limit;
		total -= limit;
	}

	printf("Loaded lots for: %s %lu\n", id.c_str(), result.size());

	return result;
}

bool checkAlert(msg::Lot& lot, msg::Alert& alert) {
	if (alert.qlt && (!lot.qlt || *lot.qlt < *alert.qlt))
		return false;

	if (alert.ptn && (!lot.ptn || *lot.ptn < *alert.ptn))
		return false;

	return true;
}

std::optional<msg::Lot> parseLot(nlohmann::json& j) {
	msg::Lot lot;
	if (j.contains("additional")) {
		auto additional = j["additional"];
		if (additional.contains("bonus_properties")) {
			std::vector<std::string> bonus_properties;
			for (auto& [_, prop]: additional["bonus_properties"].items())
				bonus_properties.push_back(prop);
			lot.bonus_properties = bonus_properties;
		}

		if (additional.contains("ptn"))
			lot.ptn = additional["ptn"].get<size_t>();

		if (additional.contains("qlt"))
			lot.qlt = additional["qlt"].get<size_t>();
	}

	if (j.contains("itemId"))
		lot.item_id = j["itemId"].get<std::string>();
	else
		return std::nullopt;

	if (j.contains("buyoutPrice"))
		lot.buyout_price = j["buyoutPrice"].get<size_t>();
	else
		return std::nullopt;

	return lot;
}

void pollLots(SCAPI& scapi, WsServer& server, const Config& config) {
	using namespace std::chrono_literals;

	while (true) {
		std::unordered_map<std::string, std::vector<size_t>> c_alerts;

		for (auto& [id, alert]: persistent::alerts) {
			if (alert.enabled)
				c_alerts[alert.item].push_back(id);

			alert.min_price = 0;
		}

		if (c_alerts.empty()) {
			std::this_thread::sleep_for(10s);
			continue;
		}

		msg::ResponseAlertItems alertItems;

		for (auto& [id, alerts_ids]: c_alerts) {
			auto active = loadAll(scapi, id);

			for (auto& lot_json: active) {
				auto lot_o = parseLot(lot_json);
				if (!lot_o) {
					std::string invalid_lot = lot_json.dump(4);
					printf("Invalid lot %s\n", invalid_lot.c_str());
					continue;
				}

				auto& lot = *lot_o;
				if (lot.buyout_price == 0)
					continue;

				for (auto& alert_id: alerts_ids) {
					auto& alert = persistent::alerts[alert_id];
					if (checkAlert(lot, alert)) {
						if (lot.buyout_price <= alert.price + 1)
							lot.alert_ids.push_back(alert_id);

						if (lot.buyout_price < alert.min_price || alert.min_price == 0)
							alert.min_price = lot.buyout_price;
					}
				}

				if (!lot.alert_ids.empty())
					alertItems.lots.push_back(lot);
			}
		}

		alertItems.alerts = persistent::alerts;
		auto connections = server.get_connections();
		for (auto conn: connections)
			if (auth.contains(conn->remote_endpoint().address().to_string()))
				sendResponse(msg::Response{alertItems}, conn);

		std::this_thread::sleep_for(config.sc_api_poll_rate);

		{
			std::stringstream ss;
			{
				cereal::JSONOutputArchive archive(ss);
				archive(persistent::alerts);
			}
			std::ofstream f("alerts.json");
			f << ss.str();
		}
	}
}

int main() {
	Config config(getExecutablePath() / "config.json");

	if (std::filesystem::exists("alerts.json")) {
		std::ifstream f("alerts.json");
		{
			cereal::JSONInputArchive archive(f);
			archive(persistent::alerts);
		}
	}

	SCAPI scapi(config.sc_api_id, config.sc_api_secret);
	getItems();

	WsServer server;
	server.config.port = port;
	auto& echo = server.endpoint["^/echo/?$"];
	echo.on_message
	  = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
			auto out_message = in_message->string();
			std::cout << "echo: \"" << out_message << "\" from " << connection.get() << std::endl;

			connection->send(out_message, [](const SimpleWeb::error_code& ec) {
				if (ec) {
					std::cout << "Server: Error sending message. " <<
					  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html
				      // Error Codes for error code meanings
					  "Error: " << ec << ", error message: " << ec.message() << std::endl;
				}
			});
		};

	auto& api = server.endpoint["^/api/?$"];
	api.on_message
	  = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
			std::stringstream ss;
			ss << in_message->string();
			msg::Request request;
			{
				cereal::JSONInputArchive archive(ss);
				archive(request);
			}
			processRequest(std::move(request), connection);
		};

	std::promise<unsigned short> server_port;
	std::thread server_thread([&server, &server_port]() {
		server.start([&server_port](unsigned short port) { server_port.set_value(port); });
	});

	std::thread polling([&] { pollLots(scapi, server, config); });

	std::cout << "Server listening on port " << server_port.get_future().get() << std::endl;
	server_thread.join();
	polling.join();
}

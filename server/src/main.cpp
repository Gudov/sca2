#include <nlohmann/json.hpp>
#include <optional>
#include <thread>
#include <utility>
#include <vector>
#include <websocket/server_ws.hpp>

#include <sstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
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

constexpr int port = 8001;

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

void update_stalcraft_git(const std::filesystem::path db_path) {
	if (!std::filesystem::exists(db_path)) {
		std::string command = "git clone https://github.com/EXBO-Studio/stalcraft-database/ " + db_path.string();
		printf("start: %s\n", command.c_str());
		int result = std::system(command.c_str());
		printf("finish git: %d\n", result);

		if (result != 0)
			throw std::runtime_error("Failed to clone the repository. Error code: " + std::to_string(result));
	} else {
		std::string command = "cd " + db_path.string() + " && git pull";
		printf("start: %s\n", command.c_str());
		int result = std::system(command.c_str());
		printf("finish git: %d\n", result);

		if (result != 0)
			throw std::runtime_error("Failed to update the repository. Error code: " + std::to_string(result));
	}
}

[[nodiscard]] std::filesystem::path getItemDB() {
	const std::filesystem::path db_path = getExecutablePath() / "db";
	update_stalcraft_git(db_path);
	return db_path;
}

namespace persistent {
std::unordered_map<size_t, msg::Alert> alerts;
}  // namespace persistent

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

std::unordered_map<std::string, std::string> getItems() {
	using namespace std::chrono_literals;
	const std::chrono::system_clock::duration period = 10s;
	static std::chrono::system_clock::time_point last;
	static bool init = false;
	const auto now = std::chrono::system_clock::now();
	bool update = false;
	if (!init) {
		init = true;
		update = true;
		last = now;
	} else if (now - last < period) {
		update = true;
	}

	static std::unordered_map<std::string, std::string> items;
	if (update) {
		items = parseDatabase(getItemDB() / "ru" / "items");
		printf("loaded: %lu items\n", items.size());
	}

	return items;
}

void processRequest(msg::Request&& request, std::shared_ptr<WsServer::Connection>& connection) {
	std::visit(
	  overloaded{
		[&](msg::RequestPing& ping) {
			printf("recieve ping: %s\n", ping.str.c_str());
			sendResponse({msg::ResponsePing{.str = ping.str}}, connection);
		},
		[&](msg::Version& ver) {
			printf("client connected, version %d %s\n", ver.build_number, ver.version.c_str());
			sendResponse(
			  {msg::Version{.build_number = BUILD_NUMBER, .version = BUILD_VERSION, .msg_hash = MSG_HASH}},
			  connection
			);
		},
		[&](msg::RequestItems& r) {
			sendResponse({msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts}}, connection);
		},
		[&](msg::RequestHistory& r) {

		},
		[&](msg::RequestRemoveAlert& alert) {
			if (persistent::alerts.contains(alert.id))
				persistent::alerts.erase(alert.id);
			sendResponse({msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts}}, connection);
		},
		[&](msg::RequestSwitchAlert& alert) {
			if (persistent::alerts.contains(alert.id))
				persistent::alerts[alert.id].enabled = alert.state;
			sendResponse({msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts}}, connection);
		},
		[&](msg::RequestAddAlert& alert) {
			persistent::alerts[alert.id] = alert.alert;
			sendResponse({msg::ResponseItems{.items = getItems(), .alerts = persistent::alerts}}, connection);
		}
	  },
	  request.request
	);
}

template<typename T>
void mapToJson(const std::unordered_map<T, T>& map, const std::filesystem::path& output_path) {
	std::ofstream(output_path) << nlohmann::json(map).dump(4);
}

void parseLots(nlohmann::json& json, std::vector<nlohmann::json>& result) {
	auto lots = json["lots"];
	size_t count = 0;
	for (auto& [_, lot]: lots.items()) {
		result.push_back(lot);
		count++;
	}
	printf("  recived %lu\n", count);
}

std::vector<nlohmann::json> loadAll(SCAPI& scapi, const std::string& id) {
	printf("request all: %s\n", id.c_str());
	int total;
	std::vector<nlohmann::json> result;
	auto active = scapi.getActiveLots(
	  {.region_id = "RU", .item_id = id, .limit = 200, .offset = 0, .additional = true},
	  Sort::Criterion::BuyoutPrice,
	  Sort::Order::Ascending
	);
	parseLots(active, result);
	total = active["total"].get<int>();

	int offset;
	total -= 200;
	offset += 200;
	while (total > 0) {
		auto active = scapi.getActiveLots(
		  {.region_id = "RU", .item_id = id, .limit = 200, .offset = size_t(offset), .additional = true},
		  Sort::Criterion::BuyoutPrice,
		  Sort::Order::Ascending
		);
		parseLots(active, result);
		total -= 200;
		offset += 200;
	}

	printf("loaded lots for: %s %lu\n", id.c_str(), result.size());

	return result;
}

bool check_alert(msg::Lot& lot, msg::Alert& alert) {
	if (lot.buyout_price > alert.price) {
		return false;
	}

	if (alert.qlt && (!lot.qlt || *lot.qlt < *alert.qlt)) {
		return false;
	}

	if (alert.ptn && (!lot.ptn || *lot.ptn < *alert.ptn)) {
		return false;
	}

	return true;
}

std::optional<msg::Lot> parseLot(nlohmann::json &j) {
	msg::Lot lot;
	if (j.contains("additional")) {
		auto additional = j["additional"];
		if (additional.contains("bonus_properties")) {
			std::vector<std::string> bonus_properties;
			for (auto &[_, prop] : additional["bonus_properties"].items()) {
				bonus_properties.push_back(prop);
			}
			lot.bonus_properties = bonus_properties;
		}

		if (additional.contains("ptn")) {
			lot.ptn = additional["ptn"].get<size_t>();
		}

		if (additional.contains("qlt")) {
			lot.qlt = additional["qlt"].get<size_t>();
		}

		//if (additional.contains("stats_random")) {
			//lot.stats_random = additional["stats_random"].get<size_t>();
		//}
	}

	if (j.contains("itemId")) {
		lot.item_id = j["itemId"].get<std::string>();
	} else {
		return std::nullopt;
	}

	if (j.contains("buyoutPrice")) {
		lot.buyout_price = j["buyoutPrice"].get<size_t>();
	} else {
		return std::nullopt;
	}

	return lot;
}

void poolingLots(SCAPI& scapi, WsServer& server, const Config& config) {
	using namespace std::chrono_literals;
	while (true) {
		std::unordered_map<std::string, std::vector<std::pair<msg::Alert, size_t>>> c_alerts;
		for (auto& [id, alert]: persistent::alerts) {
			if (alert.enabled) {
				c_alerts[alert.item].push_back({alert, id});
			}
		}

		if (c_alerts.empty()) {
			std::this_thread::sleep_for(10s);
			continue;
		}

		msg::ResponseAlertItems alertItems;

		for (auto& [id, alerts]: c_alerts) {
			auto active = loadAll(scapi, id);
			for (auto& lot_json: active) {
				auto lot_o = parseLot(lot_json);
				if (!lot_o) {
					std::string broken_lot = lot_json.dump(4);
					printf("broken lot %s\n", broken_lot.c_str());
					continue;
				}
				auto &lot = *lot_o;
				if (lot.buyout_price == 0) {
					continue;
				}
				for (auto& alert: alerts) {
					if (check_alert(lot, alert.first)) {
						lot.alert_ids.push_back(alert.second);
					}
				}
				if (!lot.alert_ids.empty()) {
					alertItems.lots.push_back(lot);
				}
			}
		}

		if (!alertItems.lots.empty()) {
			auto connections = server.get_connections();
			for (auto conn : connections) {
				sendResponse(msg::Response{alertItems}, conn);
			}
		}

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
	Config config("config.json");

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
					  // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error
					  // Codes for error code meanings
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
		// Start server
		server.start([&server_port](unsigned short port) { server_port.set_value(port); });
	});

	std::thread pooling([&] { poolingLots(scapi, server, config); });

	std::cout << "Server listening on port " << server_port.get_future().get() << std::endl;
	server_thread.join();
	pooling.join();
}

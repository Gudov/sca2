#include <nlohmann/json.hpp>
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
	Config(const std::filesystem::path& path) {
		fromJson(path);
	}
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
		printf("finish: %d\n", result);

		if (result != 0)
			throw std::runtime_error("Failed to clone the repository. Error code: " + std::to_string(result));
	} else {
		std::string command = "cd " + db_path.string() + " && git pull";
		printf("start: %s\n", command.c_str());
		int result = std::system(command.c_str());
		printf("finish: %d\n", result);

		if (result != 0)
			throw std::runtime_error("Failed to update the repository. Error code: " + std::to_string(result));
	}
}

[[nodiscard]] std::filesystem::path getItemDB() {
	const std::filesystem::path db_path = getExecutablePath() / "db";
	update_stalcraft_git(db_path);
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
			sendResponse(
			  {msg::ResponseItems{
				.items = getItems(),
				.alerts = {{"item_1_key", {true, 10}}, {"item_2_key", {false, 20}}}
			  }},
			  connection
			);
		},
		[&](msg::RequestHistory& r) {

		},
		[&](msg::RequestRemoveAlert& r) {

		},
		[&](msg::RequestSwitchAlert& r) {

		},
		[&](msg::RequestAddAlert& r) {

		}
	  },
	  request.request
	);
}

template<typename T>
void mapToJson(const std::unordered_map<T, T>& map, const std::filesystem::path& output_path) {
	std::ofstream(output_path) << nlohmann::json(map).dump(4);
}

void poolingLots(SCAPI &scapi, WsServer &server) {
	while (true) {
		
	}
}

int main() {
	Config config("config.json");
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

	std::thread pooling([&] {
		poolingLots(scapi, server);
	});

	std::cout << "Server listening on port " << server_port.get_future().get() << std::endl;
	server_thread.join();
	pooling.join();
}

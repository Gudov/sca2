#include "websocket/server_ws.hpp"
#include <sstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include "messages.hpp"
#include <future>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

constexpr int port = 8001;

void sendResponse(Response &&response, std::shared_ptr<WsServer::Connection>& connection) {
    std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(response);
	}
	std::string str = ss.str();
    connection->send(str, [](const SimpleWeb::error_code &ec) {
        if(ec) {
            std::cout << "Server: Error sending message. " <<
                // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                "Error: " << ec << ", error message: " << ec.message() << std::endl;
        }
    });
}

void processRequest(Request &&request, std::shared_ptr<WsServer::Connection>& connection) {
    std::visit(overloaded{
        [&connection] (RequestPing &ping) {
            printf("recieve ping: %s\n", ping.str.c_str());
            sendResponse({ResponsePing{.str=ping.str}}, connection);
        }
    }, request.request);
}

int main() {
    WsServer server;
    server.config.port = port;

    auto &echo = server.endpoint["^/echo/?$"];
    echo.on_message = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
        auto out_message = in_message->string();
        std::cout << "echo: \"" << out_message << "\" from " << connection.get() << std::endl;

        connection->send(out_message, [](const SimpleWeb::error_code &ec) {
            if(ec) {
                std::cout << "Server: Error sending message. " <<
                    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                    "Error: " << ec << ", error message: " << ec.message() << std::endl;
            }
        });
    };

    auto &api = server.endpoint["^/api/?$"];
    api.on_message = [](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message) {
        std::stringstream ss;
        ss << in_message->string();
        Request request;
        {
            cereal::JSONInputArchive archive(ss);
            archive(request);
        }
        processRequest(std::move(request), connection);
    };

    std::promise<unsigned short> server_port;
    std::thread server_thread([&server, &server_port]() {
        // Start server
        server.start([&server_port](unsigned short port) {
        server_port.set_value(port);
        });
    });

    std::cout << "Server listening on port " << server_port.get_future().get() << std::endl;
    server_thread.join();
}
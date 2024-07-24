#include <imgui.h>

#ifndef __EMSCRIPTEN__
#define __EMSCRIPTEN__
#endif

#include <emscripten.h>
#include <emscripten/websocket.h>

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include <cereal/archives/json.hpp>

#include <chrono>
#include <sstream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "version.hpp"
#include "messages.hpp"
#include "app.hpp"
#include "config.hpp"
#include "ws.hpp"
#include "imgui_emc.hpp"
#include "ui_main.hpp"
#include "notify.hpp"
#include "str_utils.hpp"

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

static const std::vector<std::string> ws_urls = {
  "wss://gudov.info:443/api"
  //"ws://127.0.0.1:8001/api",
  //"ws://10.0.0.12:8001/api"
};

std::unordered_set<std::string> auth;

void processResponse(msg::Response&& response) {
	std::visit(
	  overloaded{
		[](msg::ResponsePing& ping) { printf("recive response for ping %s\n", ping.str.c_str()); },
		[](msg::Version& ver) { app::server_version = std::move(ver); },
		[](msg::ResponseItems& items) {
			printf("recieve items: %lu\n", items.items.size());
			app::items = std::move(items.items);
			app::alerts = std::move(items.alerts);
		},
		[](msg::Notify& notify) { send_notify(notify); },
		[](msg::ResponseAlertItems& alert) {
			app::alerts = alert.alerts;
			using namespace std::chrono_literals;
			if (alert.lots.empty())
				return;
			auto& lot = alert.lots.front();

			for (auto& l: alert.lots)
				if (l.buyout_price < lot.buyout_price)
					lot = l;

			static std::string last_allert = "";
			static std::chrono::system_clock::time_point last_time;
			std::string alert_name = lot.item_id;
			alert_name += std::to_string(lot.buyout_price);
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			if (last_allert == alert_name && (now - last_time) < 10s)
				return;
			last_allert = alert_name;
			last_time = now;
			std::string price = PriceFormat(lot.buyout_price);
			std::string label = app::items[lot.item_id];
			if (lot.qlt) {
				label += " ";
				label += qltToStr(*lot.qlt);
			}
			send_notify(label, price, app::items[lot.item_id]);
		},
		[](msg::ResponsePassword& resp) {
			app::auth = resp.valid;
			sendRequest({msg::Version{.build_number = BUILD_NUMBER, .version = BUILD_VERSION, .msg_hash = MSG_HASH}});
			sendRequest({msg::RequestItems{}});
		}
	  },
	  response.response
	);
}

void processResponses() {
	std::lock_guard guard(ws_queue_mutex);
	while (!ws_queue.empty()) {
		std::stringstream ss;
		ss << ws_queue.front();
		ws_queue.pop();

		msg::Response response;
		{
			cereal::JSONInputArchive archive(ss);
			archive(response);
		}
		processResponse(std::move(response));
	}
}

void loop() {
	static bool net_init = false;
	processResponses();

	begin_draw();
	auto& server_msg_hash = app::server_version.msg_hash;

	if (!server_msg_hash.empty() && server_msg_hash != MSG_HASH) {
		ImGui::Text("message.hpp hash missmatch");
		ImGui::Text(
		  "server: %d %s %s",
		  app::server_version.build_number,
		  app::server_version.version.c_str(),
		  server_msg_hash.c_str()
		);
		ImGui::Text("client: %d %s %s", BUILD_NUMBER, BUILD_VERSION, MSG_HASH);
	} else if (net_init) {
		if (!app::auth) {
			static char buff[256];
			ImGui::InputText("password", buff, 255, ImGuiInputTextFlags_Password);
			if (ImGui::Button("enter"))
				sendRequest({msg::RequestPassword{buff}});
		} else {
			draw_ui();
		}
	} else {
		static int url_id = 0;
		ImGui::Text("connecting to %s", ws_urls[url_id].c_str());
		auto ws_status = get_ws_status();
		if (ws_status == WsStatus::connected) {
			net_init = true;
		} else if (ws_status == WsStatus::error || ws_status == WsStatus::closed) {
			if (url_id < (ws_urls.size() - 1))
				url_id++;
			connect_to_ws(ws_urls[url_id]);
		} else if (ws_status == WsStatus::empty) {
			connect_to_ws(ws_urls[url_id]);
		}
	}
	end_draw();
}

void init() {
	init_gl();
	init_imgui();
}

void quit() { glfwTerminate(); }

extern "C" int main(int argc, char** argv) {
	init();

	emscripten_set_main_loop(loop, 0, 1);

	quit();

	return 0;
}

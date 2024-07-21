#include <sstream>
#include <stdio.h>
#include <string>

#include "app.hpp"
#include "config.hpp"
#include "imgui.h"

#ifndef __EMSCRIPTEN__
	#define __EMSCRIPTEN__
#endif

#include <emscripten.h>
#include <emscripten/websocket.h>

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include <version.hpp>

#include <messages.hpp>
#include <cereal/archives/json.hpp>

#include "ws.hpp"
#include "imgui_emc.hpp"
#include "ui_main.hpp"

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void processResponse(msg::Response&& response) {
	std::visit(overloaded{
				   [](msg::ResponsePing& ping) {
					   printf("recive response for ping %s\n", ping.str.c_str());
				   },
				   [](msg::Version& ver) {
					   app::server_version.first = ver.build_number;
					   app::server_version.second = ver.version;
				   }},
			   response.response);
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
	if (net_init) {
		draw_ui();
	} else {
		ImGui::Text("connecting...");
		if (is_connected()) {
			sendRequest({msg::Version{.build_number = BUILD_NUMBER, .version = BUILD_VERSION}});
			net_init = true;
		}
	}
	end_draw();
}

void init() {
	init_gl();
	init_imgui();
	connect_to_ws("ws://127.0.0.1:8001/api");
}

void quit() {
	glfwTerminate();
}

extern "C" int main(int argc, char** argv) {
	init();

	emscripten_set_main_loop(loop, 0, 1);

	quit();

	return 0;
}

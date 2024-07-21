#include <sstream>
#include <stdio.h>
#include <string>

#include "config.hpp"

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
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void processResponse(Response &&response) {
	std::visit(overloaded{
		[](ResponsePing &ping) {
			printf("recive response for ping %s\n", ping.str.c_str());
		}
	}, response.response);
}

void processResponses() {
	std::lock_guard guard(ws_queue_mutex);
	while (!ws_queue.empty()) {
		std::stringstream ss;
		ss << ws_queue.front();
		ws_queue.pop();

		Response response;
		{
			cereal::JSONInputArchive archive(ss);
			archive(response);
		}
		processResponse(std::move(response));
	}
}

void loop() {
	processResponses();

	begin_draw();
	draw_ui();
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

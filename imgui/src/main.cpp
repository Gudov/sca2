#include <sstream>
#include <stdio.h>
#include <string>
#include <mutex>
#include <queue>

#ifndef __EMSCRIPTEN__
#define __EMSCRIPTEN__
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/websocket.h>
#endif

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <version.hpp>

#include <messages.hpp>
#include <cereal/archives/json.hpp>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

constexpr bool debug = true;

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
bool show_demo_window = false;
bool show_another_window = false;
int g_width;
int g_height;

EMSCRIPTEN_WEBSOCKET_T ws;

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_width, (), {
	return Module.canvas.width;
});

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_height, (), {
	return Module.canvas.height;
});

// Function called by javascript
EM_JS(void, resizeCanvas, (), {
	js_resizeCanvas();
});

std::mutex ws_queue_mutex;
std::queue<std::string> ws_queue;

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
    puts("ws: onopen");
    return EM_TRUE;
}
EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
    puts("ws: onerror");
    return EM_TRUE;
}
EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
    puts("ws: onclose");
    return EM_TRUE;
}
EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *event, void *userData) {
	std::string message((const char*)event->data, (size_t)event->numBytes);
	if (debug) {
		printf("message: %s\n", message.c_str());
	}
	{
		std::lock_guard guard(ws_queue_mutex);
		ws_queue.push(message);
	}
    return EM_TRUE;
}

void on_size_changed()
{
	glfwSetWindowSize(g_window, g_width, g_height);

	ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

void sendRequest(Request &&request) {
	std::stringstream ss;
	{
		cereal::JSONOutputArchive archive(ss);
		archive(request);
	}
	std::string str = ss.str();
	if (debug) {
		printf("request: %s\n", str.c_str());
	}
	emscripten_websocket_send_binary(ws, (void*)str.c_str(), str.size());
}

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

void loop()
{
	int width = canvas_get_width();
	int height = canvas_get_height();

	if (width != g_width || height != g_height)
	{
		g_width = width;
		g_height = height;
		on_size_changed();
	}

	glfwPollEvents();

	processResponses();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show a simple window.
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
	{
	  	static bool show_main_window = true;
		ImGui::Begin("Main window", &show_main_window);
		static float f = 0.0f;
		static int counter = 0;
		ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		std::string version = std::to_string(BUILD_NUMBER);
		version += " ";
		version += BUILD_VERSION;
		ImGui::Text("%s", version.c_str());

		if (ImGui::Button("Open socket")) {
			EmscriptenWebSocketCreateAttributes ws_attrs = {
				"ws://127.0.0.1:8001/echo",
				NULL,
				EM_TRUE
			};

			ws = emscripten_websocket_new(&ws_attrs);
			emscripten_websocket_set_onopen_callback(ws, NULL, onopen);
			emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
			emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
			emscripten_websocket_set_onmessage_callback(ws, NULL, onmessage);
		}

		if (ImGui::Button("Send text")) {
			std::stringstream ss;
			Request request{RequestPing{"ping"}};
			sendRequest(std::move(request));
		}

		if (ImGui::Button("Close")) {
			EMSCRIPTEN_RESULT result;
			result = emscripten_websocket_close(ws, 1000, "no reason");
			if (result) {
				printf("Failed to emscripten_websocket_close(): %d\n", result);
			}
		}

		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	  	ImGui::End();
	}

	// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui::Render();

	int display_w, display_h;
	glfwMakeContextCurrent(g_window);
	glfwGetFramebufferSize(g_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwMakeContextCurrent(g_window);
}


int init_gl()
{
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return 1;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	int canvasWidth = g_width;
	int canvasHeight = g_height;
	g_window = glfwCreateWindow(canvasWidth, canvasHeight, "WebGui Demo", NULL, NULL);
	if( g_window == NULL )
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(g_window); // Initialize GLEW

	return 0;
}


int init_imgui()
{
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(g_window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();

	resizeCanvas();

	return 0;
}

void connect_to_ws(const std::string &url) {
	EmscriptenWebSocketCreateAttributes ws_attrs = {
		url.c_str(),
		NULL,
		EM_TRUE
	};

	ws = emscripten_websocket_new(&ws_attrs);
	emscripten_websocket_set_onopen_callback(ws, NULL, onopen);
	emscripten_websocket_set_onerror_callback(ws, NULL, onerror);
	emscripten_websocket_set_onclose_callback(ws, NULL, onclose);
	emscripten_websocket_set_onmessage_callback(ws, NULL, onmessage);
}

int init()
{
  init_gl();
  init_imgui();
  connect_to_ws("ws://127.0.0.1:8001/api");
  return 0;
}


void quit()
{
  glfwTerminate();
}


extern "C" int main(int argc, char** argv)
{
  g_width = canvas_get_width();
  g_height = canvas_get_height();
  if (init() != 0) return 1;

  #ifdef __EMSCRIPTEN__

  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}

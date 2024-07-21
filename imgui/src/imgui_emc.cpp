#include "imgui_emc.hpp"

#include <stdio.h>
#include <string>

#include <emscripten.h>
#include <emscripten/websocket.h>

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "version.hpp"

int g_width;
int g_height;

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

void on_size_changed() {
	glfwSetWindowSize(g_window, g_width, g_height);
	ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

int init_gl() {
    g_width = canvas_get_width();
	g_height = canvas_get_height();

	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return 1;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	int canvasWidth = g_width;
	int canvasHeight = g_height;
	std::string title = "sca ";
	title += std::to_string(BUILD_NUMBER);
	g_window = glfwCreateWindow(canvasWidth, canvasHeight, title.c_str(), NULL, NULL);
	if( g_window == NULL )
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(g_window); // Initialize GLEW

	return 0;
}

int init_imgui() {
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

void begin_draw() {
    int width = canvas_get_width();
	int height = canvas_get_height();

	if (width != g_width || height != g_height)
	{
		g_width = width;
		g_height = height;
		on_size_changed();
	}

	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void end_draw() {
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
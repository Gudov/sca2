#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot/implot.h>

#include <emscripten.h>
#include <emscripten/websocket.h>

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <stdio.h>
#include <string>

#include "imgui_emc.hpp"

int g_width;
int g_height;

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_width, (), { return Module.canvas.width; });

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_height, (), { return Module.canvas.height; });

// Function called by javascript
EM_JS(void, resizeCanvas, (), { js_resizeCanvas(); });

void on_size_changed() {
	glfwSetWindowSize(g_window, g_width, g_height);
	ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

int init_gl() {
	g_width = canvas_get_width();
	g_height = canvas_get_height();

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	int canvasWidth = g_width;
	int canvasHeight = g_height;
	std::string title = "sca";
	g_window = glfwCreateWindow(canvasWidth, canvasHeight, title.c_str(), NULL, NULL);
	if (g_window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(g_window);  // Initialize GLEW

	return 0;
}

void applyImStyle() {
	static const auto bg_dark = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	static const auto bg_mid = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	static const auto accent_dark = ImVec4(0.30f, 0.30f, 0.30f, 1.000f);
	static const auto accent_light = ImVec4(0.40f, 0.40f, 0.40f, 1.000f);
	static const auto active = ImVec4(0.05f, 0.05f, 0.05f, 1.000f);
	static const auto attention = ImVec4(0.80f, 0.80f, 0.00f, 1.000f);

	auto& style = ImGui::GetStyle();
	style.WindowPadding = {6, 6};
	style.FramePadding = {6, 3};
	style.CellPadding = {6, 3};
	style.ItemSpacing = {6, 6};
	style.ItemInnerSpacing = {6, 6};
	style.ScrollbarSize = 16;
	style.GrabMinSize = 8;
	style.WindowBorderSize = style.ChildBorderSize = style.PopupBorderSize = style.TabBorderSize = 0;
	style.FrameBorderSize = 1;
	style.WindowRounding = style.ChildRounding = style.PopupRounding = style.ScrollbarRounding = style.GrabRounding
	  = style.TabRounding = 4;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
	colors[ImGuiCol_WindowBg] = bg_dark;
	colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.21f, 0.27f, 0.00f);
	colors[ImGuiCol_PopupBg] = bg_mid;
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
	colors[ImGuiCol_FrameBgHovered] = accent_light;
	colors[ImGuiCol_FrameBgActive] = active;
	colors[ImGuiCol_TitleBg] = accent_dark;
	colors[ImGuiCol_TitleBgActive] = accent_dark;
	colors[ImGuiCol_TitleBgCollapsed] = accent_dark;
	colors[ImGuiCol_MenuBarBg] = accent_dark;
	colors[ImGuiCol_ScrollbarBg] = bg_mid;
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.89f, 0.89f, 0.93f, 0.27f);
	colors[ImGuiCol_ScrollbarGrabHovered] = accent_light;
	colors[ImGuiCol_ScrollbarGrabActive] = active;
	colors[ImGuiCol_CheckMark] = accent_dark;
	colors[ImGuiCol_SliderGrab] = accent_dark;
	colors[ImGuiCol_SliderGrabActive] = accent_light;
	colors[ImGuiCol_Button] = accent_dark;
	colors[ImGuiCol_ButtonHovered] = accent_light;
	colors[ImGuiCol_ButtonActive] = active;
	colors[ImGuiCol_Header] = accent_dark;
	colors[ImGuiCol_HeaderHovered] = accent_light;
	colors[ImGuiCol_HeaderActive] = active;
	colors[ImGuiCol_Separator] = accent_dark;
	colors[ImGuiCol_SeparatorHovered] = accent_light;
	colors[ImGuiCol_SeparatorActive] = active;
	colors[ImGuiCol_ResizeGrip] = accent_dark;
	colors[ImGuiCol_ResizeGripHovered] = accent_light;
	colors[ImGuiCol_ResizeGripActive] = active;
	colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
	colors[ImGuiCol_TabHovered] = accent_light;
	colors[ImGuiCol_TabActive] = accent_dark;
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = active;
	colors[ImGuiCol_PlotLines] = accent_light;
	colors[ImGuiCol_PlotLinesHovered] = active;
	colors[ImGuiCol_PlotHistogram] = accent_light;
	colors[ImGuiCol_PlotHistogramHovered] = active;
	colors[ImGuiCol_TableHeaderBg] = accent_dark;
	colors[ImGuiCol_TableBorderStrong] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
	colors[ImGuiCol_TextSelectedBg] = accent_light;
	colors[ImGuiCol_DragDropTarget] = attention;
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
#ifdef IMGUI_HAS_DOCK
	colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
#endif

	ImPlot::StyleColorsAuto();
	ImVec4* pcolors = ImPlot::GetStyle().Colors;
	pcolors[ImPlotCol_PlotBg] = ImVec4(0, 0, 0, 0);
	pcolors[ImPlotCol_PlotBorder] = ImVec4(0, 0, 0, 0);

	pcolors[ImPlotCol_Line] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	pcolors[ImPlotCol_Fill] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

	pcolors[ImPlotCol_Selection] = attention;
	pcolors[ImPlotCol_Crosshairs] = colors[ImGuiCol_Text];
	ImPlot::GetStyle().DigitalBitHeight = 20;

	auto& pstyle = ImPlot::GetStyle();
	pstyle.PlotPadding = pstyle.LegendPadding = {12, 12};
	pstyle.LabelPadding = pstyle.LegendInnerPadding = {6, 6};
	pstyle.LegendSpacing = {10, 2};
	pstyle.AnnotationPadding = {4, 2};

	const ImU32 Dracula[] = {
	  4288967266,
	  4285315327,
	  4286315088,
	  4283782655,
	  4294546365,
	  4287429361,
	  4291197439,
	  4294830475,
	  4294113528,
	  4284106564
	};
	ImPlot::GetStyle().Colormap = ImPlot::AddColormap("Dracula", Dracula, 10);
}

#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8d9

int init_imgui() {
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(g_window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsDark();

	resizeCanvas();
	applyImStyle();

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;

	io.Fonts->Clear();

	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;

	ImFontConfig iconConfig;
	iconConfig.OversampleH = 1;
	iconConfig.OversampleV = 1;
	iconConfig.PixelSnapH = 1;

	const std::string font = "SauceCodeProNerdFont-Regular.ttf";
	static const ImWchar ranges[] = {
	  0x0020,
	  0x00FF,  // Basic Latin + Latin Supplement
	  0x0400,
	  0x044F,  // Cyrillic
	  ICON_MIN_FA,
	  ICON_MAX_FA,
	  0,
	};

	io.Fonts->AddFontFromFileTTF(font.c_str(), 15.0f, &iconConfig, ranges);

	io.Fonts->Build();
	io.Fonts->AddFontDefault();

	return 0;
}

void begin_draw() {
	int width = canvas_get_width();
	int height = canvas_get_height();

	if (width != g_width || height != g_height) {
		g_width = width;
		g_height = height;
		on_size_changed();
	}

	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static bool show_demo_window = true;
	if (show_demo_window) {
		ImGui::SetNextWindowPos(
		  ImVec2(650, 20),
		  ImGuiCond_FirstUseEver
		);  // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here
		    // we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}
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

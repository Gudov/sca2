#include <imgui.h>

#include <algorithm>
#include <string>
#include <random>

#include "version.hpp"
#include "app.hpp"
#include "ui_main.hpp"
#include "messages.hpp"
#include "ws.hpp"
#include "views/view.hpp"
#include "views/alert_table_view.hpp"
#include "views/history_view.hpp"
#include "views/item_list_view.hpp"

enum class ViewType {
	Empty,
	ItemList,
	AlertTable,
	History
};

static std::vector<std::unique_ptr<View>> views;

template<typename ViewClass>
ViewClass* createView(const ImVec2& pos, const ImVec2& size, const std::string& title) {
	static int view_count = 0;
	std::string id_title = std::format("{}##{}", title, view_count++);
	std::unique_ptr<ViewClass> view = std::make_unique<ViewClass>(true, pos, size, id_title);
	views.emplace_back(std::move(view));
	return (ViewClass*)views.back().get();
}

void createNewView(const ViewType& type) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution pos_dist_x(0, 150);
	std::uniform_int_distribution pos_dist_y(25, 150);

	const auto& position = ImVec2(pos_dist_x(gen), pos_dist_y(gen));

	switch (type) {
		case ViewType::Empty:      createView<::View>(position, ImVec2(350, 350), ""); break;
		case ViewType::ItemList:   createView<ItemListView>(position, ImVec2(300, 350), "Item list"); break;
		case ViewType::AlertTable: createView<AlertTableView>(position, ImVec2(600, 350), "Table of alerts"); break;
		case ViewType::History:    createView<HistoryView>(position, ImVec2(600, 350), "Price history"); break;
	}
}

void updateViews() {
	std::vector<View*> windows;
	windows.reserve(views.size());
	for (const auto& window: views)
		windows.emplace_back(std::move(window.get()));

	std::ranges::sort(windows, [](const View* w1, const View* w2) -> bool { return w1->last_click > w2->last_click; });

	for (const auto& window: windows)
		window->update();

	auto subrange = std::ranges::remove_if(views, [](const auto& view) { return !view->is_open; });
	views.erase(subrange.begin(), subrange.end());
}

void draw_ui() {
	auto viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags main_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar
	                              | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
	                              | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
	                              | ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (bool open = true; !ImGui::Begin("Main ", &open, main_flags)) {
		ImGui::End();
		return;
	}

	// bool prefs_open = false;

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Create")) {
			using enum ViewType;
			if (ImGui::MenuItem("Item list"))
				createNewView(ItemList);

			if (ImGui::MenuItem("Alert list"))
				createNewView(AlertTable);
			ImGui::EndMenu();
		}

		// if (ImGui::BeginMenu("Edit")) {
		// 	if (ImGui::MenuItem("Preferences"))
		// 		prefs_open = true;
		// 	ImGui::EndMenu();
		// }

		ImGui::Text("| Client: %d %s |", BUILD_NUMBER, BUILD_VERSION);
		ImGui::Text("Server: %d %s", app::server_version.build_number, app::server_version.version.c_str());

		ImGui::EndMenuBar();
	}

	// if (prefs_open)
	// 	ImGui::OpenPopup("Preferences");

	// if (ImGui::BeginPopupModal("Preferences")) {
	/*static int pollRateSec = 2;//std::chrono::duration_cast<std::chrono::seconds>(app->settings.pollRate).count();
	        ImGui::Text("Auction polling rate (seconds)");
	        ImGui::InputInt("##pollrate", &pollRateSec);

	        if (ImGui::Button("OK")) {
	                //app->settings.pollRate = std::chrono::seconds(pollRateSec);
	                ImGui::CloseCurrentPopup();
	        }

	        ImGui::SameLine();

	        if (ImGui::Button("Cancel")) {
	                ImGui::CloseCurrentPopup();
	        }*/
	// ImGui::EndPopup();
	// }

	ImGuiID dockspace_id = ImGui::GetID("Main");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	updateViews();
	ImGui::End();

	if (false) {
		static bool show_main_window = true;
		ImGui::Begin("debug", &show_main_window);
		std::string version = std::to_string(BUILD_NUMBER);
		version += " ";
		version += BUILD_VERSION;
		ImGui::Text("%s", version.c_str());
		if (ImGui::Button("Send text")) {
			std::stringstream ss;
			msg::Request request{msg::RequestPing{"ping"}};
			sendRequest(std::move(request));
		}
		ImGui::Text(
		  "Application average %.3f ms/frame (%.1f FPS)",
		  1000.0f / ImGui::GetIO().Framerate,
		  ImGui::GetIO().Framerate
		);
		ImGui::End();
	}
}

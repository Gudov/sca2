#include "ui_main.hpp"

#include <algorithm>
#include <string>
#include <random>

#include "app.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "version.hpp"
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
	static int viewCount = 0;
	std::string idTitle = std::format("{}##{}", title, viewCount++);
	std::unique_ptr<ViewClass> view = std::make_unique<ViewClass>(true, pos, size, idTitle);
	views.emplace_back(std::move(view));
	return (ViewClass*)views.back().get();
}

void CreateNewView(const ViewType& type) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution posDistX(0, 150);
	std::uniform_int_distribution posDistY(25, 150);

	const auto& position = ImVec2(posDistX(gen), posDistY(gen));

	switch (type) {
		case ViewType::Empty: {
			createView<::View>(position, ImVec2(350, 350), "");
			break;
		}

		case ViewType::ItemList: {
			createView<ItemListView>(position, ImVec2(300, 350), "Item list");
			break;
		}

		case ViewType::AlertTable: {
			createView<AlertTableView>(position, ImVec2(600, 350), "Table of alerts");
			break;
		}

		case ViewType::History: {
			createView<HistoryView>(position, ImVec2(600, 350), "Price history");
			break;
		}
	}
}

void UpdateViews() {
	// Update all views
	std::vector<View*> windows;
	windows.reserve(views.size());
	for (const auto& window : views) {
		windows.emplace_back(std::move(window.get()));
	}

	std::ranges::sort(windows, [](const View* w1, const View* w2) -> bool {
		return w1->lastClick > w2->lastClick;
	});

	for (auto const& window : windows) {
		window->Update();
	}

	auto subrange = std::ranges::remove_if(views, [](const auto& view) {
		return !view->isOpen;
	});
	views.erase(subrange.begin(), subrange.end());
}

void draw_ui() {
	auto viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (bool open = true; !ImGui::Begin("Main ", &open, windowFlags)) {
		ImGui::End();
		return;
	}

	bool prefsOpen = false;

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Create")) {
			using enum ViewType;
			if (ImGui::MenuItem("Item list")) {
				CreateNewView(ItemList);
			}

			if (ImGui::MenuItem("Alert list")) {
				CreateNewView(AlertTable);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Preferences")) {
				prefsOpen = true;
			}
			ImGui::EndMenu();
		}

        ImGui::Text("| client: %d %s |", BUILD_NUMBER, BUILD_VERSION);
	    ImGui::Text("server: %d %s", app::server_version.first, BUILD_VERSION);

		ImGui::EndMenuBar();
	}

	if (prefsOpen) {
		ImGui::OpenPopup("Preferences");
	}

	if (ImGui::BeginPopupModal("Preferences")) {
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
		ImGui::EndPopup();
	}

	ImGuiID dockspaceID = ImGui::GetID("Main");
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
	UpdateViews();
	ImGui::End();

	{
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
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

#include "ui_main.hpp"

#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "version.hpp"
#include "messages.hpp"
#include "ws.hpp"

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
            //using enum ViewType;
            if (ImGui::MenuItem("Item list")) {
                //CreateNewView(ItemList);
            }

            if (ImGui::MenuItem("Alert list")) {
                //CreateNewView(AlertTable);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Preferences")) {
                prefsOpen = true;
            }
            ImGui::EndMenu();
        }
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
    //UpdateViews();
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
			Request request{RequestPing{"ping"}};
			sendRequest(std::move(request));
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	  	ImGui::End();
	}

}
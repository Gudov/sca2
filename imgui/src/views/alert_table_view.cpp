#include "alert_table_view.hpp"

AlertTableView::AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
		View(open, pos, size, title) {}

void AlertTableView::Update() {
	auto flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin(title.c_str(), &isOpen, flags)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			lastClick = std::chrono::system_clock::now();

		ImVec2 wContentSize = ImGui::GetContentRegionAvail();
		ImGui::SameLine();

		/*RenderSearchBox(app);
				RenderAlertControls(app);
				RenderItemTable(app, wContentSize);*/
	}
	ImGui::End();
}

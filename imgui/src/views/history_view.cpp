#include "history_view.hpp"

HistoryView::HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
	View(open, pos, size, title) {
	last_day = std::chrono::system_clock::now();
}

void HistoryView::update() {
	if (ImGui::Begin(title.c_str(), &is_open)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			last_click = std::chrono::system_clock::now();
	}
	ImGui::End();
}

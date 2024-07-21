#include "history_view.hpp"

HistoryView::HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
		View(open, pos, size, title) {
	lastDay = std::chrono::system_clock::now();
}

void HistoryView::Update() {
	if (ImGui::Begin(title.c_str(), &isOpen)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			lastClick = std::chrono::system_clock::now();
	}
	ImGui::End();
}

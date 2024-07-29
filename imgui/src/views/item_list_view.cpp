#include <imgui.h>

#include "item_list_view.hpp"
#include "app.hpp"
#include "ui_main.hpp"
#include "history_view.hpp"
#include "ws.hpp"
#include "messages.hpp"
#include "str_utils.hpp"

ItemListView::ItemListView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
	View(open, pos, size, title) {}

void ItemListView::update() {
	const auto& flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin(title.c_str(), &is_open, flags)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			last_click = std::chrono::system_clock::now();

		ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::SameLine();
		ImGui::InputTextWithHint(" ", "Enter item name...", query, query_size - 1);

		if (ImGui::BeginChild("List", size, true)) {
			for (const auto& [id, name]: app::items) {
				if (contains(name, query)) {
					if (ImGui::Selectable(name.c_str())) {
						HistoryView* historyView
						  = createView<HistoryView>(ImVec2(100, 100), ImVec2(600, 350), "Price history");
						sendRequest({msg::RequestHistory{id}});
						historyView->itemID = id;
					}
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

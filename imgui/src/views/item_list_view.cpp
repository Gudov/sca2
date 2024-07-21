#include "item_list_view.hpp"

#include "app.hpp"
#include "imgui.h"
#include "ui_main.hpp"
#include "history_view.hpp"
#include "ws.hpp"
#include "messages.hpp"

namespace {

template<typename T>
[[nodiscard]] T toLower(const T& input) {
    T str = input;
    for (auto& c : str) {
        if (std::is_same_v<T, std::wstring>) {
            if (std::iswupper(static_cast<std::wint_t>(c))) {
                c = std::move(std::towlower(static_cast<std::wint_t>(c)));
            }
        } else if (std::is_same_v<T, std::string>) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                c = std::move(std::tolower(static_cast<unsigned char>(c)));
            }
        } else {
            return T();
        }
    }
    return str;
}

bool contains(const std::string& s1, const std::string& s2) {
    return toLower(s1).find(toLower(s2)) != std::string::npos;
}

}

ItemListView::ItemListView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title)
  : View(open, pos, size, title) {}

void ItemListView::Update() {
	const auto& flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin(title.c_str(), &isOpen, flags)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused()) {
			lastClick = std::chrono::system_clock::now();
		}

		ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::SameLine();
		ImGui::InputTextWithHint(" ", "Enter item name...", query, query_size - 1);

		if (ImGui::BeginChild("List", size, true)) {
			for (const auto& [id, name] : app::items) {
                if (contains(name, query)) {
                    bool hasAlert = app::alerts.contains(id);
                    if (hasAlert) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{20, 255, 20, 255});
                    }
                    if (ImGui::Selectable(name.c_str())) {
                        HistoryView* historyView = createView<HistoryView>(ImVec2(100, 100), ImVec2(600, 350), "Price history");
                        sendRequest({msg::RequestHistory{id}});
                        historyView->itemID = id;
                    }
                    if (hasAlert) {
                        ImGui::PopStyleColor(1);
                    }
                }
            }
		}
		ImGui::EndChild();
	}
	ImGui::End();
};

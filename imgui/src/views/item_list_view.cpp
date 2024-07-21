#include "item_list_view.hpp"

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
        ImGui::InputTextWithHint(" ", "Enter item name...", query, query_size-1);

        if (ImGui::BeginChild("List", size, true)) {
            /*for (const auto& [id, name] : app->itemDatabase) {
                if (contains(name, query)) {
                    bool hasAlert = app->alerts.contains(id);
                    if (hasAlert) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{20, 255, 20, 255});
                    }
                    if (ImGui::Selectable(name.c_str())) {
                        HistoryView* historyView = gui->createView<HistoryView>(ImVec2(100, 100), ImVec2(600, 350), "Price history");
                        app->fetchHistory(id);
                        historyView->itemID = id;
                    }
                    if (hasAlert) {
                        ImGui::PopStyleColor(1);
                    }
                }
            }*/
        }
        ImGui::EndChild();
    }
    ImGui::End();
};
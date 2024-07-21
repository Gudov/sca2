#include "view.hpp"

View::View(const bool& open, const ImVec2 pos, const ImVec2& size, const std::string& title)
        : isOpen(open)
        , pos(pos)
        , size(size)
        , title(title){};

void View::updateSizePos() {
    ImGui::SetWindowSize(size, ImGuiCond_Once);
    ImGui::SetWindowPos(pos, ImGuiCond_Once);
    size = ImGui::GetWindowSize();
    pos = ImGui::GetWindowPos();
};
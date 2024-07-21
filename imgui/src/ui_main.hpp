#pragma once

#include <string>

#include "imgui.h"

void draw_ui();
template<typename ViewClass>
ViewClass* createView(const ImVec2& pos, const ImVec2& size, const std::string& title);

#pragma once

#include <imgui.h>

#include <string>
#include <chrono>

class View {
  public:
	View(const bool& open, const ImVec2 pos, const ImVec2& size, const std::string& title);
	virtual void Update() {};
	void updateSizePos();

	bool isOpen = false;
	ImVec2 pos;
	ImVec2 size;
	std::string title;
	std::chrono::system_clock::time_point lastClick;
};

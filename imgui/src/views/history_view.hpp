#pragma once

#include "view.hpp"

class HistoryView : public View {
  public:
	HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

	void update() override;

	std::string itemID;

  private:
	std::vector<size_t> x;
	std::vector<size_t> y;

	std::chrono::system_clock::time_point last_day;

	int fetch_amount = 1000;
	bool show_alert = false;
};

#pragma once

#include "view.hpp"

class HistoryView : public View {
  public:
	HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

	void update() override;

	std::string item_id;
	bool is_fetching = false;
	int fetch_amount = 1000;

  private:
	struct Plot {
		std::vector<double> x;
		std::vector<double> y;
	} plot;
};

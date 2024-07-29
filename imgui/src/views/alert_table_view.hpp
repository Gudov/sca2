#pragma once

#include "view.hpp"

class AlertTableView : public View {
  public:
	AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

	void update() override;

  private:
	void RenderSearchBox();
	void RenderAlertControls();

	void RenderItemTable(const ImVec2& wContentSize);

	std::string rub;
	std::string qlt;
	std::string ptn;
	std::string percent;

	std::optional<size_t> tab_id_v;
	std::optional<size_t> alert_id_v;
	std::optional<int> rub_v;
	std::optional<int> qlt_v;
	std::optional<int> ptn_v;
	std::optional<int> percent_v;
	bool enabled_v = false;

	std::string query;
	std::string item_name;
	std::optional<std::string> item_id;
	int threshold = 0;
};

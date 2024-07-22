#include "view.hpp"

class AlertTableView : public View {
  public:
	AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

	void Update() override;

  private:
	void RenderSearchBox();
	void RenderAlertControls();
	void RenderItemTable(const ImVec2& wContentSize);

	size_t getNewId();

	std::string rub;
	std::string qlt;
	std::string ptn;
	std::string percent;

	std::optional<size_t> id_v;
	std::optional<int> rub_v;
	std::optional<int> qlt_v;
	std::optional<int> ptn_v;
	std::optional<int> percent_v;
	bool enabled_v = false;

	std::string query;
	std::string itemName;
	std::optional<std::string> itemID;
	int threshold = 0;
};

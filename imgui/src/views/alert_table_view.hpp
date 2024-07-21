#include "view.hpp"

class AlertTableView : public View {
	public:
		AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

		void Update() override;

	private:
		void RenderSearchBox();
		void RenderAlertControls();
		void RenderItemTable(const ImVec2& wContentSize) const;

	static const size_t query_size = 1024;
	char query_s[query_size];
	std::string query;
	std::string itemName;
	std::string itemID;
	int threshold = 0;
};

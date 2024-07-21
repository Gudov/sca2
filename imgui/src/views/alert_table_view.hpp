#include "view.hpp"

class AlertTableView : public View {
	public:
		AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

		void Update() override;

	private:
		void RenderSearchBox();
		void RenderAlertControls();
		void RenderItemTable(const ImVec2& wContentSize) const;

		std::string query;
		std::string itemName;
		std::string itemID;
		float threshold = 0;
};

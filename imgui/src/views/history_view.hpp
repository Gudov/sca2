#include "view.hpp"

class HistoryView : public View {
	public:
		HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title);

		void Update() override;

		std::string itemID;

	private:
		std::vector<size_t> x;
		std::vector<size_t> y;

		std::chrono::system_clock::time_point lastDay;

		int fetchAmount = 1000;
		bool showAlert = false;
};

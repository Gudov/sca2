#include <imgui.h>
#include <imgui_stdlib.h>
#include <implot/implot.h>
#include <implot/implot_internal.h>

#include <string>

#include "str_utils.hpp"
#include "history_view.hpp"
#include "app.hpp"
#include "messages.hpp"
#include "ws.hpp"

namespace {
std::vector<double> calculateVolume(
  const std::vector<double>& trade_dates,
  const std::vector<double>& trade_prices,
  const double& min_date,
  const double& max_date,
  const double& min_price,
  const double& max_price,
  const size_t& price_steps
) {
	std::vector<double> volume(price_steps, 0);
	for (size_t i = 0; i < trade_dates.size(); i++) {
		if (trade_dates[i] >= min_date && trade_dates[i] <= max_date) {
			if (trade_prices[i] < min_price) {
				volume[0]++;
			} else if (trade_prices[i] > max_price) {
				volume[price_steps - 1]++;
			} else {
				const float& priceNormal = float(trade_prices[i] - min_price) / float(max_price - min_price);
				size_t priceStepIdx = std::clamp(priceNormal * price_steps, 0.0f, float(price_steps - 1));
				volume[priceStepIdx]++;
			}
		}
	}
	return volume;
}
}  // namespace

HistoryView::HistoryView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
	View(open, pos, size, title) {}

void HistoryView::update() {
	if (ImGui::Begin(title.c_str(), &is_open)) {
		if (ImGui::IsWindowFocused())
			last_click = std::chrono::system_clock::now();

		View::updateSizePos();

		ImGui::InputInt("Amount", &fetch_amount);
		ImGui::SameLine();

		if (ImGui::Button("Fetch")) {
			if (app::history.contains(item_id))
				app::history.erase(item_id);

			is_fetching = true;
			sendRequest({msg::RequestHistory{.id = item_id, .amount = fetch_amount}});
		}

		if (app::history.contains(item_id)) {
			auto& hist_data = app::history.at(item_id);

			if (is_fetching) {
				is_fetching = false;
				plot.x.clear();
				plot.y.clear();
				for (auto rit = hist_data.rbegin(); rit != hist_data.rend(); ++rit) {
					const auto& trade = *rit;
					plot.x.push_back(trade.time.time_since_epoch().count());
					std::cout << trade.time << "\n";
					plot.y.push_back(trade.price);
				}
				ImPlot::SetNextAxesToFit();
			}
		}

		const bool plot_has_data = !(plot.x.empty() && plot.y.empty());

		if (plot_has_data) {
			double min_date = 0;
			double max_date = 0;
			double min_price = 0;
			double max_price = 0;

			ImVec2 size = ImGui::GetContentRegionAvail();
			size.x -= 100;
			if (ImPlot::BeginPlot("Historical data", size, ImPlotFlags_Crosshairs)) {
				ImPlot::SetupAxisFormat(ImAxis_X1, formatDate);
				ImPlot::SetupAxisFormat(ImAxis_Y1, formatPrice);

				ImPlot::PlotLine("Price", plot.x.data(), plot.y.data(), plot.x.size());

				min_date = ImPlot::GetCurrentPlot()->Axes[ImAxis_X1].ScaleMin;
				max_date = ImPlot::GetCurrentPlot()->Axes[ImAxis_X1].ScaleMax;
				min_price = ImPlot::GetCurrentPlot()->Axes[ImAxis_Y1].ScaleMin;
				max_price = ImPlot::GetCurrentPlot()->Axes[ImAxis_Y1].ScaleMax;

				ImPlot::EndPlot();
			}

			ImGui::SameLine();

			ImVec2 volume_size = size;
			volume_size.x = 100;
			if (ImPlot::BeginPlot("Volume Profile", volume_size, ImPlotFlags_NoLegend)) {
				const size_t steps = 20;
				std::vector<double> volume
				  = calculateVolume(plot.x, plot.y, min_date, max_date, min_price, max_price, steps);
				ImPlot::GetCurrentPlot()->Axes[ImAxis_X1].Flags
				  |= (ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Invert);
				ImPlot::GetCurrentPlot()->Axes[ImAxis_Y1].Flags
				  |= (ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_AutoFit);
				ImPlot::PlotBars("Volume Profile", volume.data(), steps, 0.8f, 0, ImPlotBarsFlags_Horizontal);
				ImPlot::EndPlot();
			}
		} else {
			ImGui::Text("%s", "No data");
		}
		ImGui::End();
	}
}


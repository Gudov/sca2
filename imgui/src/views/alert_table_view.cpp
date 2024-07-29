#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cstring>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "str_utils.hpp"
#include "app.hpp"
#include "alert_table_view.hpp"
#include "ws.hpp"
#include "messages.hpp"

namespace {

std::optional<int> inputOptionalInt(const std::string& name, std::string& v, size_t width = 50) {
	// ImGui::InputTextWithHint(name.c_str(), "...", &v, ImGuiInputTextFlags_CharsDecimal);
	// ImGui::LabelText(name.c_str(), "%s", name.c_str());
	// ImGui::SameLine();
	ImGui::SetNextItemWidth(width);
	ImGui::InputText(name.c_str(), &v, ImGuiInputTextFlags_CharsDecimal);
	if (v.empty())
		return std::nullopt;
	try {
		return std::stoi(v);
	} catch (const std::invalid_argument& ia) { return std::nullopt; }
}

std::vector<size_t> getSortedAlertIds(
  const std::unordered_set<size_t>& alerts, const std::unordered_map<size_t, msg::Alert>& app_alerts
) {
	std::vector<size_t> alert_ids;
	for (const auto& id: alerts)
		alert_ids.emplace_back(id);

	auto is_active
	  = [](const msg::Alert& alert) -> bool { return alert.min_price <= alert.price && alert.min_price != 0; };

	std::sort(alert_ids.begin(), alert_ids.end(), [&](const auto& a, const auto& b) -> bool {
		const auto& a_alert = app_alerts.at(a);
		const auto& b_alert = app_alerts.at(b);

		bool a_active = is_active(a_alert);
		bool b_active = is_active(b_alert);

		if (a_active != b_active)
			return a_active > b_active;

		return a_alert.item > b_alert.item;
	});

	return alert_ids;
}

template<typename T>
size_t getNewId(const std::unordered_map<size_t, T>& map) {
	size_t i = 0;
	while (app::tabs.contains(i))
		i++;
	return i;
}

}  // namespace

AlertTableView::AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
	View(open, pos, size, title) {}

void AlertTableView::update() {
	auto flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin(title.c_str(), &is_open, flags)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			last_click = std::chrono::system_clock::now();

		ImVec2 wContentSize = ImGui::GetContentRegionAvail();
		ImGui::SameLine();

		RenderSearchBox();
		RenderAlertControls();
		RenderItemTable(wContentSize);
	}
	ImGui::End();
}

void AlertTableView::RenderSearchBox() {
	const std::string label = "Search";

	ImGui::InputTextWithHint(label.c_str(), "Enter item name...", &query);

	ImVec2 textSize = ImGui::CalcTextSize(label.c_str());

	if (ImGui::IsItemActive() && !query.empty())
		ImGui::OpenPopup("Autocomplete");

	ImVec2 inPos = ImGui::GetItemRectMin();
	ImVec2 inSize = ImGui::GetItemRectSize();

	ImGui::SetNextWindowPos(ImVec2(inPos.x, inPos.y + inSize.y));
	ImGui::SetNextWindowSizeConstraints(ImVec2(inSize.x - textSize.x - 6, 0), ImVec2(inSize.x - textSize.x, 196));

	auto popupFlags
	  = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus;
	if (ImGui::BeginPopup("Autocomplete", popupFlags)) {
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
		for (const auto& [id, name]: app::items) {
			if (contains(name, query)) {
				bool isSelected = false;
				ImGui::Selectable(name.c_str(), &isSelected);

				if (isSelected) {
					query = name;
					item_name = name;
					item_id = id;
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}
}

void AlertTableView::RenderAlertControls() {
	ImGui::SameLine();
	const std::string submit_name = alert_id_v.has_value() ? "Edit" : "Add";
	if (ImGui::Button(submit_name.c_str()) && item_id.has_value() && rub_v.has_value()) {
		sendRequest({msg::RequestAddAlert{
		  .tab_id = tab_id_v.value_or(getNewId(app::tabs)),
		  .alert_id = alert_id_v.value_or(getNewId(app::alerts)),
		  .alert
		  = {.item = *item_id, .enabled = enabled_v, .price = (size_t)*rub_v, .qlt = qlt_v, .percent = percent_v, .ptn = ptn_v}
		}});
		tab_id_v = std::nullopt;
		alert_id_v = std::nullopt;
		rub = "";
		qlt = "";
		ptn = "";
		percent = "";
		item_id = std::nullopt;
		enabled_v = false;
		query = "";
	}

	ImGui::SetNextItemWidth(60);
	if (alert_id_v)
		ImGui::Text("id: %lu", *alert_id_v);
	else
		ImGui::Text("id: new");

	ImGui::SameLine();
	rub_v = inputOptionalInt("rub", rub, 90);
	ImGui::SameLine();
	qlt_v = inputOptionalInt("qlt", qlt);
	ImGui::SameLine();
	ptn_v = inputOptionalInt("ptn", ptn);
	ImGui::SameLine();
	percent_v = inputOptionalInt("per", percent);
}

void AlertTableView::RenderItemTable(const ImVec2& wContentSize) {
	std::optional<size_t> removedAlert = std::nullopt;
	if (ImGui::BeginTabBar("Alert Tabs", ImGuiTabBarFlags_Reorderable)) {
		for (const auto& [tab_id, tab_alert_ids]: app::tabs) {
			const std::string tab_name = "Tab " + std::to_string(tab_id);
			if (bool tab_selected = ImGui::BeginTabItem(tab_name.c_str())) {
				if (tab_selected)
					tab_id_v = tab_id;

				if (ImGui::BeginChild("Table", wContentSize, true)) {
					ImGui::BeginTable("Alerts", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
					ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed, 20);
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Qlt", ImGuiTableColumnFlags_WidthFixed, 20);
					ImGui::TableSetupColumn("+", ImGuiTableColumnFlags_WidthFixed, 20);
					ImGui::TableSetupColumn("%", ImGuiTableColumnFlags_WidthFixed, 20);
					ImGui::TableSetupColumn("rub", ImGuiTableColumnFlags_WidthFixed, 60);
					ImGui::TableSetupColumn("last", ImGuiTableColumnFlags_WidthFixed, 60);
					ImGui::TableSetupColumn("Toggle", ImGuiTableColumnFlags_WidthFixed, 60);
					ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, 60);
					ImGui::TableHeadersRow();

					std::vector<size_t> alert_ids = getSortedAlertIds(tab_alert_ids, app::alerts);

					for (auto alert_id: alert_ids) {
						std::string tracked_item_name;
						auto& alert = app::alerts[alert_id];
						if (app::items.contains(alert.item)) {
							tracked_item_name = app::items.at(alert.item);

							if (!query.empty() && !contains(tracked_item_name, query))
								continue;

						} else {
							tracked_item_name = "invalid item id ";
							tracked_item_name += alert.item;

							if (!query.empty())
								continue;
						}

						ImVec4 color;
						ImGui::TableNextRow();

						ImGui::TableNextColumn();
						std::string id_name = std::to_string(alert_id);

						if (ImGui::Button(id_name.c_str())) {
							alert_id_v = alert_id;
							item_id = alert.item;
							rub = std::to_string(alert.price);
							qlt = alert.qlt ? std::to_string(*alert.qlt) : std::string();
							ptn = alert.ptn ? std::to_string(*alert.ptn) : std::string();
							percent = alert.percent ? std::to_string(*alert.percent) : std::string();
							enabled_v = alert.enabled;
						}

						if (alert.enabled && alert.price != 0)
							if (alert.min_price > alert.price || alert.min_price == 0)
								color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
							else
								color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
						else
							color = ImVec4(100, 100, 100, 255);

						ImGui::PushStyleColor(ImGuiCol_Text, color);

						// Item name
						ImGui::TableNextColumn();
						ImGui::Text("%s", tracked_item_name.c_str());
						ImGui::PopStyleColor();

						// Qlt
						ImGui::TableNextColumn();
						if (alert.qlt) {
							ImGui::PushStyleColor(ImGuiCol_Text, qltToColor(*alert.qlt));
							ImGui::Text("%d", *alert.qlt);
							ImGui::PopStyleColor();
						}

						// +
						ImGui::TableNextColumn();
						if (alert.ptn)
							ImGui::Text("%d", *alert.ptn);

						// %
						ImGui::TableNextColumn();
						if (alert.percent) {
							ImGui::PushStyleColor(ImGuiCol_Text, qltToColor(percToQlt(*alert.percent)));
							ImGui::Text("%d", *alert.percent);
							ImGui::PopStyleColor();
						}

						// Alert threshold price
						ImGui::TableNextColumn();
						char buff[256];
						formatPrice(alert.price, buff, 250, nullptr);
						ImGui::Text("%s", buff);

						// Last price
						ImGui::TableNextColumn();
						formatPrice(alert.min_price, buff, 250, nullptr);
						ImGui::Text("%s", buff);

						// Toggle button
						ImGui::TableNextColumn();
						std::string toggleName = std::format("{}##{}", alert.enabled ? "Disable" : "Enable", alert_id);
						if (ImGui::Button(toggleName.c_str()))
							sendRequest({msg::RequestSwitchAlert{alert_id, !alert.enabled}});

						// Remove button
						std::string removeName = std::format("{}##{}", "Remove", alert_id);
						ImGui::TableNextColumn();
						if (ImGui::Button(removeName.c_str()))
							removedAlert = alert_id;
					}
					ImGui::EndTable();
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
		}
		if (ImGui::TabItemButton("+", ImGuiTabItemFlags_NoReorder | ImGuiTabItemFlags_Trailing)) {
			sendRequest({msg::RequestAddAlert{
			  .tab_id = getNewId(app::tabs),
			}});
		}
		if (removedAlert) {
			sendRequest({msg::RequestRemoveAlert{*removedAlert}});
			removedAlert = std::nullopt;
		}
	}
}

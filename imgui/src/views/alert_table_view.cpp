#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cstring>
#include <optional>
#include <vector>

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

}  // namespace

AlertTableView::AlertTableView(const bool& open, const ImVec2& pos, const ImVec2& size, const std::string& title) :
	View(open, pos, size, title) {}

void AlertTableView::Update() {
	auto flags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
	if (ImGui::Begin(title.c_str(), &isOpen, flags)) {
		View::updateSizePos();

		if (ImGui::IsWindowFocused())
			lastClick = std::chrono::system_clock::now();

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
					itemName = name;
					itemID = id;
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}
}

size_t AlertTableView::getNewId() {
	size_t i = 0;
	while (app::alerts.contains(i))
		i++;
	return i;
}

void AlertTableView::RenderAlertControls() {
	ImGui::SameLine();

	if (ImGui::Button("add/edit") && itemID.has_value() && rub_v.has_value()) {
		sendRequest({msg::RequestAddAlert{
		  .id = id_v.value_or(getNewId()),
		  .alert
		  = {.item = *itemID, .enabled = enabled_v, .price = (size_t)*rub_v, .qlt = qlt_v, .percent = percent_v, .ptn = ptn_v}
		}});
		id_v = std::nullopt;
		rub = "";
		qlt = "";
		ptn = "";
		percent = "";
		itemID = std::nullopt;
		enabled_v = false;
		query = "";
	}

	ImGui::SetNextItemWidth(60);
	if (id_v)
		ImGui::Text("id: %lu", *id_v);
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
		std::vector<size_t> alert_ids;
		for (auto& alert: app::alerts)
			alert_ids.push_back(alert.first);
		std::sort(alert_ids.begin(), alert_ids.end(), [](auto& a, auto& b) -> bool {
			return app::alerts[a].item > app::alerts[b].item;
		});
		for (auto alert_id: alert_ids) {
			auto& alert = app::alerts[alert_id];
			std::string trackedItemName;
			if (app::items.contains(alert.item)) {
				trackedItemName = app::items.at(alert.item);
				if (!query.empty() && !contains(trackedItemName, query))
					continue;
			} else {
				trackedItemName = "broken item id: ";
				trackedItemName += alert.item;
				if (!query.empty())
					continue;
			}
			ImVec4 color;
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			std::string id_name = std::to_string(alert_id);
			if (ImGui::Button(id_name.c_str())) {
				id_v = alert_id;
				itemID = alert.item;
				rub = std::to_string(alert.price);
				qlt = alert.qlt ? std::to_string(*alert.qlt) : std::string();
				ptn = alert.ptn ? std::to_string(*alert.ptn) : std::string();
				percent = alert.percent ? std::to_string(*alert.percent) : std::string();
				enabled_v = alert.enabled;
			}
			// ImGui::Text("%lu", id);

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
			ImGui::Text("%s", trackedItemName.c_str());

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
			PriceFormat(alert.price, buff, 250, nullptr);
			ImGui::Text("%s", buff);

			// last price
			ImGui::TableNextColumn();
			PriceFormat(alert.min_price, buff, 250, nullptr);
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
	}
	if (removedAlert) {
		sendRequest({msg::RequestRemoveAlert{*removedAlert}});
		removedAlert = std::nullopt;
	}
	ImGui::EndChild();
}

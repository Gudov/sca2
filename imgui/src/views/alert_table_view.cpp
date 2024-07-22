#include "alert_table_view.hpp"
#include "app.hpp"
#include "imgui_internal.h"
#include "str_utils.hpp"
#include <cstring>

#include "ws.hpp"
#include "messages.hpp"

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

	ImGui::InputTextWithHint(label.c_str(), "Enter item name...", query_s, query_size);
	query = query_s;

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
					strcpy(query_s, query.c_str());
					itemName = name;
					itemID = id;
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}
}

void AlertTableView::RenderAlertControls() {
	ImGui::SameLine();
	bool isEditingExistingAlert = app::alerts.contains(itemID);
	bool isValidInput = !itemName.empty() && threshold != 0;

	if (ImGui::Button(isEditingExistingAlert ? "Edit alert" : "Add alert") && isValidInput)
		sendRequest({msg::RequestAddAlert{.name = itemID, .price = size_t(threshold)}});

	ImGui::SameLine();
	ImGui::InputInt("Threshold##", &threshold, *"%f");
}

void AlertTableView::RenderItemTable(const ImVec2& wContentSize) const {
	std::string removedAlert;
	if (ImGui::BeginChild("Table", wContentSize, true)) {
		ImGui::BeginTable("Alerts", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Alert price", ImGuiTableColumnFlags_WidthFixed, 100);
		ImGui::TableSetupColumn("Toggle", ImGuiTableColumnFlags_WidthFixed, 60);
		ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, 60);
		ImGui::TableHeadersRow();
		for (auto& [id, alert]: app::alerts) {
			std::string trackedItemName;
			if (app::items.contains(id)) {
				trackedItemName = app::items.at(id);
			} else {
				trackedItemName = "broken item id: ";
				trackedItemName += id;
			}
			ImVec4 color;
			ImGui::TableNextRow();

			if (alert.enabled && alert.price != 0) {
				color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				// TODO
				/*if (alert.minPrice <= alert.price) {
					color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				} else {
					color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
				}*/
			} else {
				color = ImVec4(100, 100, 100, 255);
			}
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			char buff[256];

			// Item name
			ImGui::TableNextColumn();
			ImGui::Text("%s", trackedItemName.c_str());

			// Alert threshold price
			ImGui::TableNextColumn();
			PriceFormat(alert.price, buff, 250, nullptr);
			ImGui::Text("%s", buff);

			// Toggle button
			ImGui::TableNextColumn();
			std::string toggleName = std::format("{}##{}", alert.enabled ? "Disable" : "Enable", id);
			if (ImGui::Button(toggleName.c_str()))
				sendRequest({msg::RequestSwitchAlert{removedAlert, !alert.enabled}});

			// Remove button
			std::string removeName = std::format("{}##{}", "Remove", id);
			ImGui::TableNextColumn();
			if (ImGui::Button(removeName.c_str()))
				removedAlert = id;

			ImGui::PopStyleColor();
		}
		ImGui::EndTable();
	}
	if (!removedAlert.empty()) {
		sendRequest({msg::RequestRemoveAlert{removedAlert}});
		removedAlert.clear();
	}
	ImGui::EndChild();
}

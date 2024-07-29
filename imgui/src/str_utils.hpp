#pragma once

#include <imgui.h>

#include <algorithm>
#include <string>

inline bool contains(const std::string& s1, const std::string& s2) {
	const auto tolower = [](std::string data) {
		std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
		return data;
	};

	return tolower(s1).find(tolower(s2)) != std::string::npos;
}

int formatPrice(double value, char* buff, int size, void*);
std::string formatPrice(double value);
int percToQlt(float perc);

std::string qltToStr(int qlt, std::string len = "ru");
ImVec4 qltToColor(int qlt);
int percToQlt(float perc);

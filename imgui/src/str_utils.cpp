#include <ctime>
#include <sstream>
#include <iomanip>
#include <stdio.h>

#include "str_utils.hpp"

int formatDate(double value, char* buff, int size, void*) {
	time_t epoch = value / 1'000'000;
	const std::tm* t = std::localtime(&epoch);
	std::ostringstream oss;
	oss << std::put_time(t, "%d %b %H:%M:%S");
	return snprintf(buff, size, "%s", oss.str().c_str());
}

int formatPrice(double value, char* buff, int size, void*) {
	if (value >= 1'000'000)
		return snprintf(buff, size, "%.2lfM", value / 1'000'000);
	if (value >= 1'000)
		return snprintf(buff, size, "%.2lfk", value / 1'000);
	return snprintf(buff, size, "%lu", size_t(value));
}

std::string formatPrice(double value) {
	char buff[256];
	formatPrice(value, buff, 255, nullptr);
	return buff;
}

std::string qltToStr(int qlt, std::string len) {
	switch (qlt) {
		case 0:  return "обычный";
		case 1:  return "необычный";
		case 2:  return "особый";
		case 3:  return "редкий";
		case 4:  return "исключительный";
		case 5:  return "легендарный";
		default: return "wrong qlt";
	}
}

ImVec4 qltToColor(int qlt) {
	switch (qlt) {
		case 0:  return ImVec4(1, 1, 1, 1);
		case 1:  return ImVec4(0.33f, 1, 0.33f, 1);
		case 2:  return ImVec4(0.33f, 0.33f, 1, 1);
		case 3:  return ImVec4(0.58f, 0, 0.58f, 1);
		case 4:  return ImVec4(0.77f, 0.34f, 0.25f, 1);
		case 5:  return ImVec4(0.73f, 0.58f, 0.125f, 1);
		default: return ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
	}
}

int percToQlt(float perc) {
	if (perc < 100)
		return 0;
	else if (perc < 110)
		return 1;
	else if (perc < 120)
		return 2;
	else if (perc < 130)
		return 3;
	else if (perc < 140)
		return 4;
	else
		return 5;
}

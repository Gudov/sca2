#pragma once

#include <string>
#include <unordered_map>

#include "messages.hpp"

namespace app {

extern std::pair<int, std::string> server_version;
extern std::unordered_map<std::string, std::string> items;
extern std::unordered_map<std::string, msg::Alert> alerts;

}	 // namespace app

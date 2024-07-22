#pragma once

#include <string>
#include <unordered_map>

#include "messages.hpp"

namespace app {

extern msg::Version server_version;
extern std::unordered_map<std::string, std::string> items;
extern std::unordered_map<size_t, msg::Alert> alerts;

}	 // namespace app

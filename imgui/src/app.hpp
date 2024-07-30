#pragma once

#include <string>
#include <unordered_map>

#include "messages.hpp"

namespace app {

extern msg::Version server_version;
extern std::unordered_map<std::string, std::string> items;
extern std::unordered_map<size_t, msg::Alert> alerts;
extern std::unordered_map<size_t, std::unordered_set<size_t>> tabs;
extern std::unordered_map<std::string, std::vector<msg::History>> history;

extern bool auth;

}  // namespace app

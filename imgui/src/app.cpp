#include "app.hpp"
#include "messages.hpp"

namespace app {

msg::Version server_version;
std::unordered_map<std::string, std::string> items;
std::unordered_map<size_t, msg::Alert> alerts;
std::unordered_map<size_t, std::unordered_set<size_t>> tabs;
std::unordered_map<std::string, std::vector<msg::History>> history;

bool auth;

}  // namespace app

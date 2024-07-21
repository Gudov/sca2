#include "app.hpp"
#include <unordered_map>

namespace app {

std::pair<int, std::string> server_version;
std::unordered_map<std::string, std::string> items;
std::unordered_map<std::string, msg::Alert> alerts;

}	 // namespace app

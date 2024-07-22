#include "app.hpp"
#include "messages.hpp"
#include <unordered_map>

namespace app {

msg::Version server_version;
std::unordered_map<std::string, std::string> items;
std::unordered_map<std::string, msg::Alert> alerts;

}	 // namespace app

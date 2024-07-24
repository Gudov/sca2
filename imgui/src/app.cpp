#include <unordered_map>

#include "app.hpp"
#include "messages.hpp"

namespace app {

msg::Version server_version;
std::unordered_map<std::string, std::string> items;
std::unordered_map<size_t, msg::Alert> alerts;
bool auth;

}  // namespace app

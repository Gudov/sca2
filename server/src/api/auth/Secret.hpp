#pragma once

#include <cpr/cprtypes.h>

#include <string>

namespace Auth {
class Secret {
  public:
	Secret(const std::string& url, const std::string& client_id, const std::string& client_secret);

	[[nodiscard]] std::string getToken() const;

  private:
	std::string endpoint;
	std::string client_id;
	std::string client_secret;
};
}  // namespace Auth

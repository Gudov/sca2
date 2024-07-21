#include "ServerErrorException.hpp"

ServerErrorException::ServerErrorException(const std::string& message) : std::runtime_error(message){};

void checkStatusCode(const cpr::Response& r) {
	if (r.status_code != 200) {
		std::string error
		  = std::format("[{}:{}]\nStatus code: {}\n{}", __builtin_FUNCTION(), __builtin_LINE(), r.status_code, r.text);
		throw ServerErrorException(error);
	}
}

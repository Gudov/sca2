#include <iostream>

#include "ServerErrorException.hpp"

ServerErrorException::ServerErrorException(const std::string& message) : std::runtime_error(message){};

void checkStatusCode(const cpr::Response& r) {
	if (r.status_code != 200) {
		std::cout << "Error Code: " << static_cast<int>(r.error.code) << '\n';
		std::cout << "Error Message: " << r.error.message << '\n' << std::endl;
		std::string error
		  = std::format("[{}:{}]\nStatus code: {}\n{}", __builtin_FUNCTION(), __builtin_LINE(), r.status_code, r.text);
		throw ServerErrorException(error);
	}
}

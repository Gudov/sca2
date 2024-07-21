#pragma once

#include <cpr/response.h>

#include <stdexcept>

class ServerErrorException : public std::runtime_error {
  public:
	explicit ServerErrorException(const std::string& message);
};

void checkStatusCode(const cpr::Response& r);

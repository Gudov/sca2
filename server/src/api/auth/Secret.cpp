#include <nlohmann/json.hpp>

#include <cpr/payload.h>
#include <cpr/response.h>
#include <cpr/api.h>

#include "Secret.hpp"
#include "api/ServerErrorException.hpp"

Auth::Secret::Secret(const std::string& url, const std::string& client_id, const std::string& client_secret) :
	endpoint(url),
	client_id(client_id),
	client_secret(client_secret) {}

std::string Auth::Secret::getToken() const {
	cpr::Header header = {{"Content-Type", "application/x-www-form-urlencoded"}};
	cpr::Payload payload{
	  {"client_id", this->client_id},
	  {"client_secret", this->client_secret},
	  {"grant_type", "client_credentials"},
	  {"scope", ""}
	};

	cpr::Response r = cpr::Post(cpr::Url{this->endpoint}, header, payload);
	checkStatusCode(r);
	return nlohmann::json::parse(r.text)["access_token"];
}

#include <cpr/ssl_options.h>

#include "SCAPI.hpp"
#include "api/ServerErrorException.hpp"

namespace {

inline const std::string exbo_oauth_url = "https://exbo.net/oauth/token/";
inline const std::string stalcraft_api_url = "https://eapi.stalcraft.net/";

template<typename T>
[[nodiscard]] std::string to_string(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

template<typename... Args>
[[nodiscard]] cpr::Url constructUrl(const std::string& baseURL, const Args&... args) {
	std::string url = baseURL;
	url.reserve(url.size() + sizeof...(args) * 10);

	if (!url.empty() && url.back() != '/')
		url += "/";

	for (const auto& arg: {to_string(args)...})
		url += arg + "/";

	return cpr::Url{std::move(url)};
}

std::string getToken(const std::string& client_id, const std::string& client_secret) {
	cpr::Header header = {{"Content-Type", "application/x-www-form-urlencoded"}};
	cpr::Payload payload{
	  {"client_id", client_id},
	  {"client_secret", client_secret},
	  {"grant_type", "client_credentials"},
	  {"scope", ""}
	};

	cpr::Response r = cpr::Post(cpr::Url{exbo_oauth_url}, header, payload, cpr::VerifySsl(0));
	checkStatusCode(r);
	return nlohmann::json::parse(r.text)["access_token"];
}

}  // namespace

namespace Sort {
std::string to_string(const Sort::Criterion& criterion) {
	switch (criterion) {
		case Sort::Criterion::TimeCreated:  return "time_created";
		case Sort::Criterion::TimeLeft:     return "time_left";
		case Sort::Criterion::CurrentPrice: return "current_price";
		case Sort::Criterion::BuyoutPrice:  return "buyout_price";
		default:                            return "";
	}
}

std::string to_string(const Sort::Order& order) {
	switch (order) {
		case Sort::Order::Ascending:  return "asc";
		case Sort::Order::Descending: return "desc";
		default:                      return "";
	}
}
}  // namespace Sort

SCAPI::SCAPI(const std::string& client_id, const std::string& client_secret) {
	this->token = getToken(client_id, client_secret);
	this->endpoint = stalcraft_api_url;
}

cpr::Response SCAPI::get(const cpr::Url& url, const cpr::Parameters& params) const {
	cpr::Header header{{"Content-Type", "application/json"}};
	cpr::Response r = cpr::Get(url, params, header, cpr::Bearer(token), cpr::VerifySsl(0));
	checkStatusCode(r);
	return r;
}

nlohmann::json SCAPI::getSoldLots(const Query& query) const {
	cpr::Url url = constructUrl(endpoint.str(), query.region_id, "auction", query.item_id, "history");
	cpr::Parameters params{
	  {"offset", std::to_string(query.offset)},
	  {"limit", std::to_string(query.limit)},
	  {"additional", query.additional ? "true" : "false"},
	};

	nlohmann::json data = nlohmann::json::parse(get(url, params).text);
	return data;
}

nlohmann::json
  SCAPI::getActiveLots(const Query& query, const Sort::Criterion& criteria, const Sort::Order& order) const {
	cpr::Url url = constructUrl(endpoint.str(), query.region_id, "auction", query.item_id, "lots");
	cpr::Parameters params{
	  {"offset", std::to_string(query.offset)},
	  {"limit", std::to_string(query.limit)},
	  {"sort", Sort::to_string(criteria)},
	  {"order", Sort::to_string(order)},
	  {"additional", query.additional ? "true" : "false"},
	};

	nlohmann::json data = nlohmann::json::parse(get(url, params).text);
	return data;
}

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

	cpr::Response r = cpr::Post(cpr::Url{exbo_oauth_url}, header, payload);
	checkStatusCode(r);
	return nlohmann::json::parse(r.text)["access_token"];
}

}  // namespace

namespace Sort {
std::string to_string(const Sort::Criterion& criterion) {
	static const std::unordered_map<Sort::Criterion, std::string> map = {
	  {Sort::Criterion::TimeCreated, "time_created"},
	  {Sort::Criterion::TimeLeft, "time_left"},
	  {Sort::Criterion::CurrentPrice, "current_price"},
	  {Sort::Criterion::BuyoutPrice, "buyout_price"}
	};

	auto it = map.find(criterion);
	return it != map.end() ? it->second : "";
}

std::string to_string(const Sort::Order& order) {
	static const std::unordered_map<Sort::Order, std::string> map
	  = {{Sort::Order::Ascending, "asc"}, {Sort::Order::Descending, "desc"}};

	auto it = map.find(order);
	return it != map.end() ? it->second : "";
}
}  // namespace Sort

SCAPI::SCAPI(const std::string& client_id, const std::string& client_secret) {
	this->token = getToken(client_id, client_secret);
	this->endpoint = stalcraft_api_url;
}

cpr::Response SCAPI::get(const cpr::Url& url, const cpr::Parameters& params) const {
	cpr::Header header{{"Content-Type", "application/json"}};
	cpr::Response r = cpr::Get(url, params, header, cpr::Bearer(token));
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


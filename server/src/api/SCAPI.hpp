#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <string>

struct Query {
	std::string region_id;
	std::string item_id;
	size_t limit = 20;
	size_t offset = 0;
	bool additional = false;
};

namespace Sort {
enum class Order {
	None,
	Ascending,
	Descending
};

enum class Criterion {
	None,
	TimeCreated,
	TimeLeft,
	CurrentPrice,
	BuyoutPrice,
};
std::string to_string(const Criterion& criterion);
std::string to_string(const Order& order);
}  // namespace Sort

class SCAPI {
  public:
	SCAPI(const std::string& url, const std::string& token);
	[[nodiscard]] cpr::Response get(const cpr::Url& url, const cpr::Parameters& params) const;
	[[nodiscard]] nlohmann::json getSoldLots(const Query& query) const;
	[[nodiscard]] nlohmann::json getActiveLots(
	  const Query& query,
	  const Sort::Criterion& criteria = Sort::Criterion::None,
	  const Sort::Order& order = Sort::Order::None
	) const;

  private:
	std::string token;
	cpr::Url endpoint;
};

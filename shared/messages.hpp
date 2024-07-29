#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>

#include <cstddef>
#include <string>

#include <unordered_map>
#include <optional>
#include <variant>
#include <vector>

namespace msg {

struct Alert {
	std::string item;
	bool enabled;
	bool matched;
	size_t min_price;
	size_t price;
	std::optional<int> qlt;
	std::optional<int> percent;
	std::optional<int> ptn;

	template<class Archive>
	void save(Archive& ar) const {
		ar(item, enabled, matched, min_price, price, qlt, percent, ptn);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(item, enabled, matched, min_price, price, qlt, percent, ptn);
	}
};

struct RequestPassword {
	std::string password;

	template<class Archive>
	void save(Archive& ar) const {
		ar(password);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(password);
	}
};

struct ResponsePassword {
	bool valid;

	template<class Archive>
	void save(Archive& ar) const {
		ar(valid);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(valid);
	}
};

struct RequestPing {
	std::string str;

	template<class Archive>
	void save(Archive& ar) const {
		ar(str);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(str);
	}
};

struct Version {
	int build_number;
	std::string version;
	std::string msg_hash;

	template<class Archive>
	void save(Archive& ar) const {
		ar(build_number, version, msg_hash);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(build_number, version, msg_hash);
	}
};

struct RequestItems {
	bool e;

	template<class Archive>
	void save(Archive& ar) const {
		ar(e);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(e);
	}
};

struct RequestHistory {
	std::string name;

	template<class Archive>
	void save(Archive& ar) const {
		ar(name);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(name);
	}
};

struct RequestRemoveAlert {
	size_t id;

	template<class Archive>
	void save(Archive& ar) const {
		ar(id);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(id);
	}
};

struct RequestSwitchAlert {
	size_t id;
	bool state;

	template<class Archive>
	void save(Archive& ar) const {
		ar(id, state);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(id, state);
	}
};

struct RequestAddAlert {
	size_t tab_id;
	size_t alert_id;
	Alert alert;

	template<class Archive>
	void save(Archive& ar) const {
		ar(tab_id, alert_id, alert);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(tab_id, alert_id, alert);
	}
};

struct Request {
	std::variant<
	  RequestPing,
	  Version,
	  RequestItems,
	  RequestHistory,
	  RequestRemoveAlert,
	  RequestSwitchAlert,
	  RequestAddAlert,
	  RequestPassword>
	  request;

	template<class Archive>
	void save(Archive& ar) const {
		ar(request);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(request);
	}
};

struct ResponseItems {
	std::unordered_map<std::string, std::string> items;
	std::unordered_map<size_t, Alert> alerts;
	std::unordered_map<size_t, std::unordered_set<size_t>> tabs;

	template<class Archive>
	void save(Archive& ar) const {
		ar(items, alerts, tabs);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(items, alerts, tabs);
	}
};

struct Lot {
	std::optional<std::vector<std::string>> bonus_properties;
	std::optional<std::size_t> ptn;
	std::optional<std::size_t> qlt;
	std::optional<std::size_t> stats_random;
	size_t buyout_price;
	std::string item_id;
	std::vector<size_t> alert_ids;

	template<class Archive>
	void save(Archive& ar) const {
		ar(bonus_properties, ptn, qlt, stats_random, buyout_price, item_id, alert_ids);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(bonus_properties, ptn, qlt, stats_random, buyout_price, item_id, alert_ids);
	}
};

struct ResponseAlertItems {
	std::vector<Lot> lots;
	std::unordered_map<size_t, Alert> alerts;
	std::unordered_map<size_t, std::unordered_set<size_t>> tabs;

	template<class Archive>
	void save(Archive& ar) const {
		ar(lots, alerts, tabs);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(lots, alerts, tabs);
	}
};

struct ResponsePing {
	std::string str;

	template<class Archive>
	void save(Archive& ar) const {
		ar(str);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(str);
	}
};

struct Notify {
	std::string label, body, clip;

	template<class Archive>
	void save(Archive& ar) const {
		ar(label, body, clip);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(label, body, clip);
	}
};

struct Response {
	std::variant<ResponsePing, Version, ResponseItems, Notify, ResponseAlertItems, ResponsePassword> response;

	template<class Archive>
	void save(Archive& ar) const {
		ar(response);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(response);
	}
};

}  // namespace msg

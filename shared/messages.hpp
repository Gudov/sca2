#pragma once

#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/archives/binary.hpp>
#include <unordered_map>
#include <optional>
#include <variant>

namespace msg {

struct Alert {
	std::string item;
	bool enabled;
	size_t price;
	std::optional<int> qlt;
	std::optional<int> percent;
	std::optional<int> ptn;

	template<class Archive>
	void save(Archive& ar) const {
		ar(item, enabled, price, qlt, percent, ptn);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(item, enabled, price, qlt, percent, ptn);
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
	size_t id;
	Alert alert;

	template<class Archive>
	void save(Archive& ar) const {
		ar(id, alert);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(id, alert);
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
	  RequestAddAlert>
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

	template<class Archive>
	void save(Archive& ar) const {
		ar(items, alerts);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(items, alerts);
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
	std::variant<ResponsePing, Version, ResponseItems, Notify> response;

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

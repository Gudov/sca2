#pragma once

#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/binary.hpp>
#include <unordered_map>
#include <variant>

namespace msg {

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

	template<class Archive>
	void save(Archive& ar) const {
		ar(build_number, version);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(build_number, version);
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

struct RequestSwitchAlert {
	std::string name;
	bool state;

	template<class Archive>
	void save(Archive& ar) const {
		ar(name, state);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(name, state);
	}
};

struct RequestAddAlert {
	std::string name;
	size_t price;

	template<class Archive>
	void save(Archive& ar) const {
		ar(name, price);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(name, price);
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

struct Alert {
	bool enabled;
	size_t price;

	template<class Archive>
	void save(Archive& ar) const {
		ar(enabled, price);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(enabled, price);
	}
};

struct ResponseItems {
	std::unordered_map<std::string, std::string> items;
	std::unordered_map<std::string, Alert> alerts;

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

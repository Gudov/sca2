#pragma once

#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/archives/binary.hpp>
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

struct Request {
	std::variant<RequestPing, Version> request;

	template<class Archive>
	void save(Archive& ar) const {
		ar(request);
	}

	template<class Archive>
	void load(Archive& ar) {
		ar(request);
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

struct Response {
	std::variant<ResponsePing, Version> response;

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

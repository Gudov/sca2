#pragma once

#include <string>
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/archives/binary.hpp>
#include <variant>

struct RequestPing {
    std::string str;

    template <class Archive>
    void save( Archive & ar ) const {
        ar( str );
    }

    template <class Archive>
    void load( Archive & ar ) {
        ar( str );
    }
};

struct Request {
    std::variant<RequestPing> request;

    template <class Archive>
    void save( Archive & ar ) const {
        ar( request );
    }

    template <class Archive>
    void load( Archive & ar ) {
        ar( request );
    }
};

struct ResponsePing {
    std::string str;

    template <class Archive>
    void save( Archive & ar ) const {
        ar( str );
    }

    template <class Archive>
    void load( Archive & ar ) {
        ar( str );
    }
};

struct Response {
    std::variant<ResponsePing> response;

    template <class Archive>
    void save( Archive & ar ) const {
        ar( response );
    }

    template <class Archive>
    void load( Archive & ar ) {
        ar( response );
    }
};


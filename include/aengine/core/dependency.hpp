#pragma once

// erslib
#include <erslib/exception.hpp>
#include <erslib/adaptor/transparent/string.hpp>
#include <erslib/convert/string.hpp>
#include <erslib/hashing/rapid.hpp>
#include <erslib/type/result.hpp>
#include <erslib/type/version.hpp>

// aengine
#include <aengine/fwd.hpp>


namespace aengine {
    enum class DependencyType : uint8_t {
        None = 0,
        Incompatible,
        Optional,
        Required
    };

    enum class DependencyLimit : uint8_t {
        None = 0,
        Lower,
        Upper
    };


    struct dependency_t {
        std::string name;
        DependencyType type = DependencyType::None;
        DependencyLimit limit = DependencyLimit::None;
        ers::version_t version = { 0, 0, 0 };
    };


    using DependencyContainer = HashSet<
        dependency_t,
        ers::member_string_hash_adaptor<ers::RapidHash, &dependency_t::name>,
        ers::member_equal_adaptor<&dependency_t::name>
    >;


    ERS_MAKE_EXCEPTION_TYPE(dependency_error, std::runtime_error);
}


template<>
struct ers::convert::from_string_backend<aengine::dependency_t> {
    Result<aengine::dependency_t> runtime_value(std::string_view source) const;
};

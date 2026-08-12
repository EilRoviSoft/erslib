#pragma once

// ers
#include <erslib/aengine/fwd.hpp>
#include <erslib/core/adaptor.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/convert/string.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/version.hpp>


namespace aengine::impl {
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


    using DependencyContainer = ers::HashSet<
        dependency_t,
        ers::adaptor::member_string_hash<ers::RapidHash, &dependency_t::name>,
        ers::adaptor::member_equal<&dependency_t::name>
    >;


    ERS_MAKE_EXCEPTION_TYPE(dependency_error, std::runtime_error);
}


// Exports

namespace aengine {
    using impl::DependencyType;
    using impl::DependencyLimit;
    using impl::dependency_t;
    using impl::DependencyContainer;

    using impl::dependency_error;
    using impl::make_dependency_error;
    using impl::make_dependency_error_with_trace;
}


// Specialization

template<>
struct ers::convert::from_string_backend<aengine::impl::dependency_t> {
    Result<aengine::impl::dependency_t> runtime_value(std::string_view source) const;
};

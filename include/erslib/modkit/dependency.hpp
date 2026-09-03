#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/adaptor.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/version.hpp>
#include <erslib/modkit/fwd.hpp>


namespace ers::modkit::impl {
    enum class EDependencyType : u8 {
        None = 0,
        Incompatible,
        Optional,
        Required
    };

    enum class EDependencyLimit : u8 {
        None = 0,
        Lower,
        Upper
    };


    struct dependency_t {
        std::string name;
        EDependencyType type = EDependencyType::None;
        EDependencyLimit limit = EDependencyLimit::None;
        version_t version = { 0, 0, 0 };


        // Parses "[?!]<name>[ (<|<=|>|>=|min|max) <version>]".
        static Result<dependency_t> parse(std::string_view source);
    };


    using DependencyContainer = HashSet<
        dependency_t,
        adaptor::member_string_hash<RapidHash, &dependency_t::name>,
        adaptor::member_equal<&dependency_t::name>
    >;


    ERS_MAKE_EXCEPTION_TYPE(dependency_error, std::runtime_error);
}


// Exports

namespace ers::modkit {
    using impl::EDependencyType;
    using impl::EDependencyLimit;
    using impl::dependency_t;
    using impl::DependencyContainer;

    using impl::dependency_error;
    using impl::make_dependency_error;
    using impl::make_dependency_error_with_trace;
}


#pragma once

// std
#include <type_traits>

// ers
#include <erslib/contrib/json/concept.hpp>
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/meta.hpp>
#include <erslib/core/type/result.hpp>


// from json backend

namespace utl::convert {
    template<typename T>
    struct from_json_backend;


    template<typename T>
    struct from_json_options;
}


// Traits

namespace utl::impl {
    template<typename T>
    concept FromJsonHasValue = requires(convert::from_json_backend<T> backend, const Node& source) {
        { backend.value(source) } -> std::convertible_to<ers::Result<T>>;
    };

    template<typename T>
    concept FromJsonWithOptionsHasValue = requires(
        convert::from_json_backend<T> backend,
        const Node& source,
        convert::from_json_options<T> options
    ) {
            { backend.value(source, options) } -> std::convertible_to<ers::Result<T>>;
        };
}


// Utility functions

namespace utl::impl {
    template<typename T>
    [[nodiscard]]
    ers::Result<T> from_json(const Node& source, const convert::from_json_options<T>& options = {}) {
        static_assert(FromJsonWithOptionsHasValue<T> || FromJsonHasValue<T>, "'from_json' doesn't have implementation for 'T'.");


        convert::from_json_backend<T> backend;

        if constexpr (FromJsonWithOptionsHasValue<T>) {
            return backend.value(source, options);
        } else {
            return backend.value(source);
        }
    }
}


// Implementations

template<utl::impl::JsonCompatible T>
struct utl::convert::from_json_backend<T> {
    [[nodiscard]]
    ers::Result<T> value(const impl::Node& source) const {
        if (!source.is<T>()) {
            return ers::make_error("Expected '{}' but got '{}'.",
                ers::meta::type_name_v<T>, source.type());
        }

        return source.as<T>();
    }
};

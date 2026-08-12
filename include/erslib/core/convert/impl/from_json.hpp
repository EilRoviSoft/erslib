#pragma once

// std
#include <type_traits>

// contrib
#include <erslib/contrib/json.hpp>
#include <erslib/core/concept/json.hpp>
#include <erslib/core/meta/type_name.hpp>
#include <erslib/core/type/result.hpp>


// from json backend

namespace ers::convert {
    template<typename T>
    struct from_json_backend;


    template<typename T>
    struct from_json_options;
}


// Traits

namespace ers::impl::convert {
    template<typename T>
    concept FromJsonHasValue = requires(ers::convert::from_json_backend<T> backend, const utl::Json& source) {
        { backend.value(source) } -> std::convertible_to<Result<T>>;
    };

    template<typename T>
    concept FromJsonWithOptionsHasValue = requires(
        ers::convert::from_json_backend<T> backend,
        const utl::Json& source,
        ers::convert::from_json_options<T> options
    ) {
        { backend.value(source, options) } -> std::convertible_to<Result<T>>;
    };
}


// Utility functions

namespace ers::impl::convert {
    template<typename T>
    [[nodiscard]]
    Result<T> from_json(const utl::Json& source, const ers::convert::from_json_options<T>& options = {}) {
        static_assert(FromJsonWithOptionsHasValue<T> || FromJsonHasValue<T>, "'from_json' doesn't have implementation for 'T'.");


        ers::convert::from_json_backend<T> backend;

        if constexpr (FromJsonWithOptionsHasValue<T>) {
            return backend.value(source, options);
        } else {
            return backend.value(source);
        }
    }
}


// Implementations

template<ers::JsonCompatible T>
struct ers::convert::from_json_backend<T> {
    [[nodiscard]]
    Result<T> value(const utl::Json& source) const {
        if (!source.is<T>()) {
            return ers::make_error(
                Severity::Error,
                "Expected '{}' but got '{}'.",
                ers::meta::type_name_v<T>, source.type()
            );
        }

        return source.as<T>();
    }
};

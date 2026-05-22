#pragma once

// std
#include <type_traits>

// ers
#include <contrib/json.hpp>
#include <erslib/concept/json.hpp>
#include <erslib/meta/type_name.hpp>
#include <erslib/type/exception.hpp>
#include <erslib/type/result.hpp>


// From json definitions

namespace ers::convert {
    template<typename T>
    struct from_json_backend;


    template<typename T>
    struct from_json_options;
}


// Traits

namespace ers::internal {
    template<typename T>
    concept FromJsonHasValue = requires(convert::from_json_backend<T> backend, const utl::Json& source) {
        { backend.value(source) } -> std::convertible_to<Result<T>>;
    };

    template<typename T>
    concept FromJsonWithOptionsHasValue = requires(
        convert::from_json_backend<T> backend,
        const utl::Json& source,
        convert::from_json_options<T> options
    ) {
        { backend.value(source, options) } -> std::convertible_to<Result<T>>;
    };
}


// Utility functions

namespace ers::convert {
    template<typename T>
    [[nodiscard]]
    Result<T> from_json(const utl::Json& source, const from_json_options<T>& options = {}) {
        static_assert(internal::FromJsonWithOptionsHasValue<T> || internal::FromJsonHasValue<T>, "'from_json' doesn't have implementation for 'T'.");


        from_json_backend<T> backend;

        if constexpr (internal::FromJsonWithOptionsHasValue<T>) {
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
            return Unexpected<Error>(
                Severity::Error,
                "json_type_error",
                "Expected '{}' but got '{}'.",
                ers::meta::type_name_v<T>, source.type()
            );
        }

        return source.as<T>();
    }
};

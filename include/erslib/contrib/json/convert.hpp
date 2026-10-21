#pragma once

// std
#include <concepts>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    class IJsonConvertible {
    public:
        virtual ers::Status load_from(const Json& json) = 0;
        virtual ers::Status save_into(Json& json) const = 0;


    protected:
        ~IJsonConvertible() = default;
    };


    template<std::derived_from<IJsonConvertible> T>
    static ers::Result<T> load_from_json(const Json& json) {
        T out;
        ERS_TRY(out.load_from(json));
        return out;
    }

    template<std::derived_from<IJsonConvertible> T>
    static ers::Result<Json> save_into_json(const T& what) {
        Json out;
        if (auto s = what.save_into(out); !s)
            return s.error();
        return out;
    }
}

#pragma once

// std
#include <concepts>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    class IJsonConvertible {
    public:
        virtual ers::Status load_from(const Node& json) = 0;
        virtual ers::Status save_into(Node& json) const = 0;


    protected:
        ~IJsonConvertible() = default;
    };


    template<std::derived_from<IJsonConvertible> T>
    static ers::Result<T> load_from_json(const Node& json) {
        T out;
        if (auto s = out.load_from(json); !s)
            return s.error();
        return out;
    }

    template<std::derived_from<IJsonConvertible> T>
    static ers::Result<Node> save_into_json(const T& what) {
        Node out;
        if (auto s = what.save_into(out); !s)
            return s.error();
        return out;
    }
}

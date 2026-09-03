#pragma once

// std
#include <memory>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/core/type/result.hpp>


namespace ers::luaschema::impl {
    using Descriptor = std::polymorphic<class IDescriptor>;

    class IDescriptor {
    public:
        // Member functions

        IDescriptor() = default;

        virtual ~IDescriptor() = default;


        // Executors

        virtual Status verify(sol::table table) const = 0;
    };


    template<typename T, typename... Args>
    Descriptor make_descriptor(Args&&... args) {
        return Descriptor(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}

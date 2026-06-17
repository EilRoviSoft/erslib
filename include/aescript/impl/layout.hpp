#pragma once

// std
#include <initializer_list>
#include <string_view>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/type/result.hpp>

// aengine
#include <aengine/fwd.hpp>

// aescript
#include <aescript/impl/field.hpp>


namespace aescript {
    class Layout {
    public:
        // Member functions

        Layout() = default;
        Layout(std::initializer_list<std::pair<std::string_view, Field>> il);


        // Modifiers

        void add_field(std::string name, Field field);


        // Accessors

        template<typename K>
        Field& operator[](const K& name) {
            return _fields[name];
        }
        template<typename K>
        const Field& operator[](const K& name) const {
            return _fields.at(name);
        }


        // Checkers

        [[nodiscard]]
        ers::Status verify(sol::table table) const;


    private:
        aengine::StringMap<Field> _fields;
    };


    using LayoutContainer = aengine::StringMap<Layout>;
}

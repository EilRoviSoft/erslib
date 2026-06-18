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
        Layout(std::initializer_list<Field> il);


        // Modifiers

        void add_field(Field field);


        // Accessors

        template<typename K>
        const Field& get(const K& name) const {
            return *_fields.find(name);
        }


        // Checkers

        [[nodiscard]]
        ers::Status verify(sol::table table) const;

        [[nodiscard]]
        ers::Status parse(sol::table table, void* where) const;


    private:
        aengine::HashSet<
            Field,
            ers::member_string_hash_adaptor<ers::RapidHash, &Field::name>,
            ers::member_equal_adaptor<&Field::name>
        > _fields;
    };


    using LayoutContainer = aengine::StringMap<Layout>;
}

#pragma once

// std
#include <initializer_list>
#include <variant>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/modkit/fwd.hpp>
#include <erslib/luaschema/impl/descriptor.hpp>
#include <erslib/luaschema/impl/field.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::luaschema::impl {
    class Layout {
    public:
        using possible_property_t = std::variant<Field, Descriptor>;

        // Member functions

        Layout() = default;
        Layout(std::initializer_list<possible_property_t> il);


        // Modifiers

        void add_field(Field field);

        void add_property(Descriptor property);


        // Accessors

        template<typename K>
        const Field& get(const K& name) const {
            return *_fields.find(name);
        }


        // Executors

        Status verify(sol::table table) const;

        Status parse(sol::table table, void* where) const;


    private:
        HashSet<
            Field,
            adaptor::member_string_hash<ers::RapidHash, &Field::name>,
            adaptor::member_equal<&Field::name>
        > _fields;

        std::vector<Descriptor> _descriptors;
    };


    using LayoutContainer = StringMap<Layout>;
}

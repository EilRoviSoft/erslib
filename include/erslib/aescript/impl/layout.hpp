#pragma once

// std
#include <initializer_list>
#include <variant>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/aengine/fwd.hpp>
#include <erslib/aescript/impl/descriptor.hpp>
#include <erslib/aescript/impl/field.hpp>
#include <erslib/core/type/result.hpp>


namespace aescript::impl {
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

        [[nodiscard]]
        ers::Status verify(sol::table table) const;

        [[nodiscard]]
        ers::Status parse(sol::table table, void* where) const;


    private:
        ers::HashSet<
            Field,
            ers::adaptor::member_string_hash<ers::RapidHash, &Field::name>,
            ers::adaptor::member_equal<&Field::name>
        > _fields;

        std::vector<Descriptor> _descriptors;
    };


    using LayoutContainer = ers::StringMap<Layout>;
}

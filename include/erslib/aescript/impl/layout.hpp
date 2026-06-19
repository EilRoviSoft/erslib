#pragma once

// std
#include <initializer_list>
#include <variant>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/core/type/result.hpp>

// aengine
#include <erslib/aengine/fwd.hpp>

// aescript
#include <erslib/aescript/impl/field.hpp>
#include <erslib/aescript/impl/descriptor.hpp>


namespace aescript {
    class Layout {
    public:
        using possible_property_t = std::variant<Field, DescriptorPtr>;

        // Member functions

        Layout() = default;
        Layout(std::initializer_list<possible_property_t> il);


        // Modifiers

        void add_field(Field field);

        void add_property(DescriptorPtr property);


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
        aengine::HashSet<
            Field,
            ers::member_string_hash_adaptor<ers::RapidHash, &Field::name>,
            ers::member_equal_adaptor<&Field::name>
        > _fields;

        std::vector<DescriptorPtr> _descriptors;
    };


    using LayoutContainer = aengine::StringMap<Layout>;
}

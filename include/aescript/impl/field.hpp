#pragma once

// std
#include <list>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/type/result.hpp>

// aescript
#include <aescript/impl/property.hpp>


namespace aescript {
    class Field {
        using storage_iterator = std::list<FieldPropertyPtr>::const_iterator;


    public:
        // Member functions

        explicit Field() = default;

        Field(const Field& other);
        Field& operator=(const Field& other);

        Field(Field&&) = default;
        Field& operator=(Field&&) = default;


        friend Field& operator|(Field& lhs, FieldPropertyPtr rhs);
        friend Field&& operator|(Field&& lhs, FieldPropertyPtr rhs);


        // Modifiers

        void add(FieldPropertyPtr ptr);


        // Checkers

        [[nodiscard]]
        ers::Status verify(sol::table table, std::string_view field) const;


    private:
        std::list<FieldPropertyPtr> _storage;
        std::vector<storage_iterator> _inorder;

        void _copy_from(const Field& other);
    };
}

#pragma once

// std
#include <list>
#include <memory>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/aescript/impl/parser.hpp>
#include <erslib/aescript/impl/verifier.hpp>
#include <erslib/core/type/result.hpp>


// Field

namespace aescript::impl {
    class Field {
        using storage_iterator = std::list<Verifier>::const_iterator;


    public:
        // Member functions

        explicit Field(std::string name);

        Field(const Field& other);
        Field& operator=(const Field& other);

        Field(Field&&) = default;
        Field& operator=(Field&&) = default;


        // Accessors

        std::string_view name() const { return _name; }


        // Modifiers

        void add(Verifier verifier);
        void add(Parser parser);


        // Executors

        [[nodiscard]]
        ers::Status verify(sol::table table) const;

        // Should be called only after 'verify'.
        [[nodiscard]]
        ers::Status parse(sol::table table, void* where) const;


    private:
        std::string _name;

        std::list<Verifier> _verifiers;
        std::vector<storage_iterator> _verifiers_order;

        std::list<Parser> _parsers;


        void _copy_from(const Field& other);
    };
}


// Operators

namespace aescript::impl {
    Field& operator|(Field& lhs, Verifier rhs);
    Field&& operator|(Field&& lhs, Verifier rhs);


    Field& operator|(Field& lhs, Parser rhs);
    Field&& operator|(Field&& lhs, Parser rhs);
}

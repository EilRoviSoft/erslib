#pragma once

// std
#include <list>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/type/result.hpp>

// aescript
#include <aescript/impl/parser.hpp>
#include <aescript/impl/verifier.hpp>


// Field

namespace aescript {
    class Field {
        using storage_iterator = std::list<VerifierPtr>::const_iterator;


    public:
        // Member functions

        explicit Field() = default;

        Field(const Field& other);
        Field& operator=(const Field& other);

        Field(Field&&) = default;
        Field& operator=(Field&&) = default;


        // Modifiers

        void add(VerifierPtr ptr);
        void add(ParserPtr ptr);


        // Api

        [[nodiscard]]
        ers::Status verify(sol::table table, std::string_view field) const;

        [[nodiscard]]
        ers::Status parse(sol::table table, std::string_view field, void* where) const;


    private:
        std::list<VerifierPtr> _verifiers;
        std::vector<storage_iterator> _verifiers_order;

        std::list<ParserPtr> _parsers;


        void _copy_from(const Field& other);
    };
}


// Operators

namespace aescript {
    Field& operator|(Field& lhs, VerifierPtr rhs);
    Field&& operator|(Field&& lhs, VerifierPtr rhs);


    Field& operator|(Field& lhs, ParserPtr rhs);
    Field&& operator|(Field&& lhs, ParserPtr rhs);
}

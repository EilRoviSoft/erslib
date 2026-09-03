#pragma once

// std
#include <list>
#include <memory>
#include <vector>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/impl/verifier.hpp>
#include <erslib/core/type/result.hpp>


// Field

namespace ers::luaschema::impl {
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

        Status verify(sol::table table) const;

        // Should be called only after 'verify'.
        Status parse(sol::table table, void* where) const;


    private:
        std::string _name;

        std::list<Verifier> _verifiers;
        std::vector<storage_iterator> _verifiers_order;

        std::list<Parser> _parsers;


        void _copy_from(const Field& other);
    };
}


// Operators

namespace ers::luaschema::impl {
    Field& operator|(Field& lhs, Verifier rhs);
    Field&& operator|(Field&& lhs, Verifier rhs);


    Field& operator|(Field& lhs, Parser rhs);
    Field&& operator|(Field&& lhs, Parser rhs);
}

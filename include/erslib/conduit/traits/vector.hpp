#pragma once

// std
#include <vector>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/traits.hpp>


template<typename T>
struct conduit::impl::sql_value<std::vector<T>> {
    static void bind(pqxx::params& out, const std::vector<T>& what) {
        std::string literal = "{";

        for (size_t i = 0; i < what.size(); i++) {
            if (i) literal += ',';
            literal += std::format("{}", what[i]);
        }

        out.append(literal += '}');
    }
};

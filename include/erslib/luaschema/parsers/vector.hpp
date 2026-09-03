#pragma once

// std
#include <vector>

// ers
#include <erslib/luaschema/impl/context.hpp>
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/parsers/innate.hpp>


template<typename T>
struct ers::luaschema::parser_t<std::vector<T>> {
    Status exec(impl::parser_context& ctx, sol::object obj, std::vector<T>& dst) const {
        if (!obj.is<sol::table>())
            return make_error("Expected array table");

        auto arr = obj.as<sol::table>();
        dst.reserve(arr.size());

        parser_t<T> elem_parser;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (auto s = elem_parser.exec(ctx, arr[i + 1], dst.emplace_back()); !s)
                return s;
        }

        return ok;
    }
};

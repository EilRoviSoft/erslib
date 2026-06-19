#pragma once

// std
#include <vector>

// aescript
#include <erslib/aescript/impl/context.hpp>
#include <erslib/aescript/impl/parser.hpp>
#include <erslib/aescript/parsers/innate.hpp>


template<typename T>
struct aescript::parser_t<std::vector<T>> {
    [[nodiscard]]
    ers::Status exec(parser_context& ctx, sol::object obj, std::vector<T>& dst) const {
        if (!obj.is<sol::table>())
            return ers::make_error(ers::Severity::Error, "Expected array table");

        auto arr = obj.as<sol::table>();
        dst.reserve(arr.size());

        parser_t<T> elem_parser;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (auto s = elem_parser.exec(ctx, arr[i + 1], dst.emplace_back()); !s)
                return s;
        }

        return ers::ok;
    }
};

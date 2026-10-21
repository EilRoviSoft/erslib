#pragma once

// std
#include <concepts>
#include <string_view>
#include <utility>

// ers
#include <erslib/core/trait/container.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/result/collector.hpp>
#include <erslib/flux/result/row.hpp>


template<typename T>
    requires flux::impl::ReadableRow<typename T::value_type>
    && ers::HintInsertable<T>
    && (!std::convertible_to<T, std::string_view>)
struct flux::impl::sql_collector<T> {
    using row_t = typename T::value_type;

    static T collect(const pqxx::result& from) {
        T out;
        if constexpr (ers::Reservable<T>)
            out.reserve(from.size());
        for_each_row<row_t>(from, [&](row_t&& v) { out.insert(out.end(), std::move(v)); });
        return out;
    }
};

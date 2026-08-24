#pragma once

// ers
#include <erslib/core/trait/container.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/generator.hpp>
#include <erslib/dbio/impl/traits.hpp>


template<typename T>
    requires dbio::impl::ReadableRow<typename T::value_type>
    && ers::HintInsertable<T>
    && (!std::convertible_to<T, std::string_view>)
struct dbio::impl::sql_collector<T> {
    using row_t = typename T::value_type;

    static ers::Result<T> collect(const pqxx::result& from) {
        T out;
        if constexpr (ers::Reservable<T>)
            out.reserve(from.size());
        for_each_row<row_t>(from, [&](row_t&& v) { out.insert(out.end(), std::move(v)); });
        return out;
    }
};

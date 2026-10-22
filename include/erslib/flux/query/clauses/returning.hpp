#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/flux/query/clauses/column.hpp>
#include <erslib/flux/query/slots/returning.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl::clauses {
    template<ColumnItem... Args>
    std::vector<Clause> returning(Args&&... args) {
        return make_columns(&slots::returning, std::forward<Args>(args)...);
    }

    template<ColumnNames R>
    std::vector<Clause> returning(R&& names) {
        return make_columns(&slots::returning, std::forward<R>(names));
    }

    ERSLIB_EXPORT Clause returning_all();
}

#pragma once


// Includes

#include <erslib/core/enum/reflect.hpp>
#include <erslib/core/enum/case_styles.hpp>


// Exports

namespace ers::enum_utils {
    using impl::case_style_traits;

    using impl::kebab_case_t;
    using impl::snake_case_t;
    using impl::upper_snake_case_t;

    using impl::camel_case_t;
    using impl::pascal_case_t;

    using impl::lower_case_t;
    using impl::upper_case_t;
}

namespace ers::enum_utils {
    using impl::enum_utils::as_is_t;
    using impl::enum_utils::as_is;

    using impl::enum_utils::custom_t;
    using impl::enum_utils::custom;

    using impl::enum_utils::to_string;
    using impl::enum_utils::from_string;
}

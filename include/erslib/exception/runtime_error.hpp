#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_FN(runtime_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_FN(range_error, std::range_error);
    ERS_MAKE_EXCEPTION_FN(overflow_error, std::overflow_error);
    ERS_MAKE_EXCEPTION_FN(underflow_error, std::underflow_error);

    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(parse_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(conversion_error, std::runtime_error);
}

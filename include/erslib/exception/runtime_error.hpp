#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_TYPE(runtime_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE(range_error, std::range_error);
    ERS_MAKE_EXCEPTION_TYPE(overflow_error, std::overflow_error);
    ERS_MAKE_EXCEPTION_TYPE(underflow_error, std::underflow_error);

    ERS_MAKE_EXCEPTION_TYPE(parse_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE(conversion_error, std::runtime_error);
}

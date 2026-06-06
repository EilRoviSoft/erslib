#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_FN(logic_error, std::logic_error);
    ERS_MAKE_EXCEPTION_FN(invalid_argument_error, std::invalid_argument);
    ERS_MAKE_EXCEPTION_FN(domain_error, std::domain_error);
    ERS_MAKE_EXCEPTION_FN(length_error, std::length_error);
    ERS_MAKE_EXCEPTION_FN(out_of_range_error, std::out_of_range);
}

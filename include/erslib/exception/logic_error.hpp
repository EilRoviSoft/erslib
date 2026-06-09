#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_TYPE(logic_error, std::logic_error);
    ERS_MAKE_EXCEPTION_TYPE(invalid_argument_error, std::invalid_argument);
    ERS_MAKE_EXCEPTION_TYPE(domain_error, std::domain_error);
    ERS_MAKE_EXCEPTION_TYPE(length_error, std::length_error);
    ERS_MAKE_EXCEPTION_TYPE(out_of_range_error, std::out_of_range);

    ERS_MAKE_EXCEPTION_TYPE(enumerator_error, std::out_of_range);
}

#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(logic_error, std::logic_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(invalid_argument_error, std::invalid_argument);
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(domain_error, std::domain_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(length_error, std::length_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(out_of_range_error, std::out_of_range);

    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(enumerator_error, std::out_of_range);
}

#pragma once


// Includes

#include <erslib/core/exception/logic_error.hpp>
#include <erslib/core/exception/runtime_error.hpp>


// Exports

namespace ers {
    ERS_MAKE_EXCEPTION_EXPORTS(impl, logic_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, invalid_argument_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, domain_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, length_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, out_of_range_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, format_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, enumerator_error);

    ERS_MAKE_EXCEPTION_EXPORTS(impl, runtime_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, range_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, overflow_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, underflow_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, parse_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, conversion_error);
}

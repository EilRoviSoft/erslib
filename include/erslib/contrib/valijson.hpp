#pragma once

#ifndef _HAS_VALIJSON
#error "erslib/contrib/valijson.hpp needs valijson: add the 'valijson' vcpkg port"
#endif


// Includes --------------------------------------------------------------------

#include <erslib/contrib/json.hpp>
#include <erslib/contrib/valijson/validator.hpp>


// Exports ---------------------------------------------------------------------

namespace utl {
    using impl::JsonValidator;
}

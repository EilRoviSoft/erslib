#pragma once

#ifndef _HAS_INJA
#error "erslib/contrib/inja.hpp needs inja: add the 'inja' vcpkg port"
#endif

#ifdef INCLUDE_INJA_JSON_HPP_
#error "<inja/...> was included before erslib/contrib/inja.hpp, so inja is already bound to nlohmann::json"
#endif


// Includes --------------------------------------------------------------------

#include <erslib/contrib/json.hpp>
#include <erslib/contrib/inja/compat.hpp>


// Binding ---------------------------------------------------------------------

#define INCLUDE_INJA_JSON_HPP_

namespace inja {
    using json = ::utl::impl::InjaJson;
}

#include <inja/inja.hpp>


// Exports ---------------------------------------------------------------------

namespace utl {
    using impl::inja_traits;
    using impl::InjaJson;
}

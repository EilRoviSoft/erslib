#pragma once

#if __cpp_lib_optional >= 202506L


// std
#include <optional>


namespace ers::impl {
    using std::optional;


    using nullopt_t = std::nullopt_t;
    inline constexpr nullopt_t nullopt = std::nullopt;
}


#elifdef _HAS_BOOST_OPTIONAL


// boost
#include <boost/optional.hpp>


namespace ers::impl {
    using boost::optional;


    using nullopt_t = boost::none_t;
    inline constexpr nullopt_t nullopt { nullopt_t::init_tag {} };
}


#else

#error "Your dependency inclusion doesn't support optional in required way (with T&)"

#endif


// Exports

namespace ers {
    using impl::optional;

    using impl::nullopt_t;
    using impl::nullopt;
}

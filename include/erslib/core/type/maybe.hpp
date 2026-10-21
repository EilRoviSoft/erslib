#pragma once

// std
#include <version>


#if __cpp_lib_optional >= 202506L


// std
#include <optional>


namespace ers::impl {
    template<typename T>
    using maybe = std::optional<T>;


    using none_t = std::nullopt_t;
    inline constexpr none_t none = std::nullopt;
}


#elifdef _HAS_BOOST_OPTIONAL


// boost
#include <boost/optional.hpp>


namespace ers::impl {
    template<typename T>
    using maybe = boost::optional<T>;


    using none_t = boost::none_t;
    inline constexpr none_t none { none_t::init_tag {} };
}


#else

#error "Your dependency inclusion doesn't support optional in required way (with T&)"

#endif


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::maybe;

    using impl::none_t;
    using impl::none;
}

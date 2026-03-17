#pragma once

#if __cpp_lib_optional >= 202506L


// std
#include <optional>


namespace ers {
    using std::optional;


    using nullopt_t = std::nullopt_t;
    static constexpr nullopt_t nullopt;
}


#elifdef _HAS_BOOST_OPTIONAL


// boost
#include <boost/optional.hpp>


namespace ers {
    using boost::optional;

    
    using nullopt_t = boost::none_t;
    static constexpr nullopt_t nullopt { nullopt_t::init_tag {} };
}


#else

#error "Your dependency inclusion doesn't support optional in required way (with T&)"

#endif

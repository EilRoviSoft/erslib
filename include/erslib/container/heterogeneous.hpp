#pragma once

// std
#include <string>

// ers
#include <erslib/util/string.hpp>


// boost
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>

namespace ers {
    template<
        template<typename> typename HashEngine = hashing::Std,
        typename Alloc = std::allocator<std::string>>
    using StringSet = boost::unordered_set<
        std::string,
        util::string_hash<HashEngine>,
        util::string_equal,
        Alloc
    >;
}

namespace ers {
    template<
        typename Val,
        template<typename> typename HashEngine = hashing::Std,
        typename Alloc = std::allocator<std::string>>
    using StringMap = boost::unordered_map<
        std::string,
        Val,
        util::string_hash<HashEngine>,
        util::string_equal,
        Alloc
    >;
}


// boost
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

namespace ers {
    template<
        template<typename> typename HashEngine = hashing::Std,
        typename Alloc = std::allocator<std::string>>
    using FlatStringSet = boost::unordered_set<
        std::string,
        util::string_hash<HashEngine>,
        util::string_equal,
        Alloc
    >;
}

namespace ers {
    template<
        typename Val,
        template<typename> typename HashEngine = hashing::Std,
        typename Alloc = std::allocator<std::string>>
    using FlatStringMap = boost::unordered_map<
        std::string,
        Val,
        util::string_hash<HashEngine>,
        util::string_equal,
        Alloc
    >;
}

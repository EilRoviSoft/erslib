#pragma once

// ers
#include <erslib/hashing/rapid.hpp>

// boost
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_set.hpp>

// Uses buckets as implementation

namespace ers {
    template<
        typename K,
        typename Hash = hashing::Rapid<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using UnorderedSet = boost::unordered_set<K, Hash, Equal, Alloc>;
}

namespace ers {
    template<
        typename K,
        typename V,
        typename Hash = hashing::Rapid<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using UnorderedMap = boost::unordered_map<K, V, Hash, Equal, Alloc>;
}

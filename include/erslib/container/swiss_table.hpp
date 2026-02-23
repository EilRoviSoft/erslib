#pragma once

// ers
#include <erslib/hashing/rapid.hpp>

// absl
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>


// Uses swiss tables as implementation

namespace ers {
    template<
        typename K,
        typename Hash = RapidHash<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using SwissFlatSet = absl::flat_hash_set<K, Hash, Equal, Alloc>;

    template<
        typename K,
        typename V,
        typename Hash = RapidHash<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using SwissFlatMap = absl::flat_hash_map<K, V, Hash, Equal, Alloc>;
}

namespace ers {
    template<
        typename K,
        typename Hash = RapidHash<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
        using SwissNodeSet = absl::node_hash_set<K, Hash, Equal, Alloc>;

    template<
        typename K,
        typename V,
        typename Hash = RapidHash<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using SwissNodeMap = absl::node_hash_map<K, V, Hash, Equal, Alloc>;
}

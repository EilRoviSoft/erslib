#pragma once

// absl
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

// ers
#include <erslib/hashing/rapid.hpp>

// Uses swiss table as implementation

namespace ers {
    template<
        typename K,
        typename Hash = hashing::Rapid<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using SwissSet = absl::flat_hash_set<K, Hash, Equal, Alloc>;
}

namespace ers {
    template<
        typename K,
        typename V,
        typename Hash = hashing::Rapid<K>,
        typename Equal = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using SwissMap = absl::flat_hash_map<K, V, Hash, Equal, Alloc>;
}

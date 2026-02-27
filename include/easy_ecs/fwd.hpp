#pragma once

// ers
#include <erslib/container/unordered_table.hpp>
#include <erslib/hashing/direct.hpp>
#include <erslib/memory/pmr/any.hpp>


namespace ecs {
    template<
        typename K,
        typename Hasher = ers::RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using HashSet = ers::UnorderedSet<K, Hasher, EqualTo, Alloc>;

    template<
        typename K,
        typename V,
        typename Hasher = ers::RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using HashMap = ers::UnorderedMap<K, V, Hasher, EqualTo, Alloc>;


    using TrivialSet = HashSet<size_t, ers::DirectHash<size_t>>;

    template<typename V>
    using TrivialMap = HashMap<size_t, V, ers::DirectHash<size_t>>;


    template<typename V>
    using StringMap = HashMap<std::string, V, ers::util::string_hash_adaptor<ers::RapidHash>, ers::util::string_equal>;
}


namespace ecs {
    using Object = ers::pmr::Any;
}

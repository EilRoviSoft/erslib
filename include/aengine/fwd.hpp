#pragma once

// std
#include <string>

// boost
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

// ers
#include <erslib/adaptor/transparent/string.hpp>
#include <erslib/hashing/direct.hpp>
#include <erslib/memory/any.hpp>


namespace aengine {
    template<
        typename K,
        typename Compare = std::less<K>,
        typename AllocatorOrContainer = std::allocator<K>>
    using OrderedSet = boost::container::flat_set<K, Compare, AllocatorOrContainer>;

    template<
        typename K,
        typename V,
        typename Compare = std::less<K>,
        typename AllocatorOrContainer = std::allocator<std::pair<K, V>>>
    using OrderedMap = boost::container::flat_map<K, V, Compare, AllocatorOrContainer>;
}


namespace aengine {
    template<
        typename K,
        typename Hasher = ers::RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using HashSet = boost::unordered_flat_set<K, Hasher, EqualTo, Alloc>;

    template<
        typename K,
        typename V,
        typename Hasher = ers::RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using HashMap = boost::unordered_flat_map<K, V, Hasher, EqualTo, Alloc>;


    using TrivialSet = HashSet<size_t, ers::DirectHash<size_t>>;

    template<typename V>
    using TrivialMap = HashMap<size_t, V, ers::DirectHash<size_t>>;


    using StringSet = HashSet<
        std::string,
        ers::string_hash_adaptor<ers::hashing::rapid_policy>,
        ers::equal_adaptor<std::string>
    >;

    template<typename V>
    using StringMap = HashMap<
        std::string, V,
        ers::string_hash_adaptor<ers::hashing::rapid_policy>,
        ers::equal_adaptor<std::string>
    >;


    using StringViewSet = HashSet<
        std::string_view,
        ers::string_hash_adaptor<ers::hashing::rapid_policy>,
        ers::equal_adaptor<std::string_view>
    >;

    template<typename V>
    using StringViewMap = HashMap<
        std::string_view, V,
        ers::string_hash_adaptor<ers::hashing::rapid_policy>,
        ers::equal_adaptor<std::string_view>
    >;
}


namespace aengine {
    using Object = ers::Any;
}

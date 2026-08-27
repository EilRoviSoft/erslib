#pragma once

// std
#include <string>

// boost
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

// ers
#include <erslib/core/adaptor.hpp>
#include <erslib/core/hashing/direct.hpp>
#include <erslib/core/hashing/rapid.hpp>


namespace ers::impl {
    template<
        typename K,
        typename Compare = std::less<K>,
        typename AllocatorOrContainer = std::allocator<K>>
    using OrderedSet = boost::container::flat_set<K, Compare, AllocatorOrContainer>;

    template<
        typename K,
        auto Member,
        typename Compare = std::less<>,
        typename AllocatorOrContainer = std::allocator<K>>
    using OrderedSetByMember = boost::container::flat_set<
        K,
        adaptor::member_compare<Member, Compare>,
        AllocatorOrContainer
    >;

    template<
        typename K,
        typename V,
        typename Compare = std::less<K>,
        typename AllocatorOrContainer = std::allocator<std::pair<K, V>>>
    using OrderedMap = boost::container::flat_map<K, V, Compare, AllocatorOrContainer>;
}


namespace ers::impl {
    template<
        typename K,
        typename Hasher = RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<K>>
    using HashSet = boost::unordered_flat_set<K, Hasher, EqualTo, Alloc>;

    template<
        typename K,
        auto Member,
        template<typename> typename Hasher = RapidHash,
        typename Alloc = std::allocator<K>>
    using HashSetByMember = boost::unordered_flat_set<
        K,
        adaptor::member_hash<Hasher, Member>,
        adaptor::member_equal<Member>,
        Alloc
    >;

    template<
        typename K,
        typename V,
        typename Hasher = RapidHash<K>,
        typename EqualTo = std::equal_to<K>,
        typename Alloc = std::allocator<std::pair<K, V>>>
    using HashMap = boost::unordered_flat_map<K, V, Hasher, EqualTo, Alloc>;


    using TrivialSet = HashSet<size_t, DirectHash<size_t>>;

    template<typename V>
    using TrivialMap = HashMap<size_t, V, DirectHash<size_t>>;


    using StringSet = HashSet<
        std::string,
        adaptor::string_hash<RapidHash>,
        adaptor::equal<std::string>
    >;

    template<typename V>
    using StringMap = HashMap<
        std::string, V,
        adaptor::string_hash<RapidHash>,
        adaptor::equal<std::string>
    >;
}


// Exports

namespace ers {
    using impl::OrderedSet;
    using impl::OrderedSetByMember;
    using impl::OrderedMap;

    using impl::HashSet;
    using impl::HashSetByMember;
    using impl::HashMap;

    using impl::TrivialSet;
    using impl::TrivialMap;

    using impl::StringSet;
    using impl::StringMap;
}

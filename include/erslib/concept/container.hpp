#pragma once

// std
#include <utility>
#include <type_traits>

// ers
#include <erslib/hashing/base.hpp>

namespace ers {
    template<typename T>
    concept ContiguousContainerConcept = requires(T t) {
        { t.data() } -> std::is_pointer; // Must have .data() returning a pointer
        { t.size() } -> size_t;          // Must have .size() returning an unsigned int
    };

    template<typename T>
    concept HashMapConcept =
        requires(const T t) {
            typename T::key_type;
            typename T::value_type;
            typename T::mapped_type;
            typename T::size_type;
            typename T::iterator;
            typename T::const_iterator;
        } && requires(T m, typename T::key_type k, typename T::mapped_type v) {
            { m[k] } -> std::same_as<typename T::mapped_type&>;
            { m.at(k) } -> std::same_as<typename T::mapped_type&>;
            { m.insert(std::pair { k, v }) } -> std::same_as<std::pair<typename T::iterator, bool>>;
            { m.erase(k) } -> std::same_as<typename T::size_type>;
            { m.find(k) } -> std::same_as<typename T::iterator>;
            { m.empty() } -> std::same_as<bool>;
            { m.size() } -> std::same_as<typename T::size_type>;
            { m.clear() } -> std::same_as<void>;
        };
}

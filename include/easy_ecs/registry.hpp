#pragma once

// std
#include <concepts>
#include <memory>

// ers
#include <erslib/concept/string_convertible.hpp>
#include <erslib/container/swiss_table.hpp>
#include <erslib/hashing/rapid.hpp>
#include <erslib/memory/pmr/holder.hpp>
#include <erslib/util/string.hpp>

namespace ecs {
    template<typename T>
    using Holder = ers::pmr::Holder<T>;

    template<typename T>
    class Registry {
    public:
        template<typename K, typename D>
            requires ers::StringConvertibleConcept<K>
        T& insert(const K& k, D v) {
            return _add(k, ers::pmr::make_holder<D>(std::pmr::get_default_resource(), std::move(v)));
        }

        template<typename K, typename D, typename... Args>
            requires ers::StringConvertibleConcept<K> && std::derived_from<D, T>
        T& emplace(const K& k, Args&&... args) {
            return insert(k, std::forward<Args>(args)...);
        }

        template<typename K, typename D>
            requires ers::StringConvertibleConcept<K>
        T& insert_with_allocator(const K& k, std::pmr::memory_resource* resource, D v) {
            return _add(k, ers::pmr::make_holder<D>(resource, std::move(v)));
        }
        template<typename K, typename D, typename... Args>
            requires ers::StringConvertibleConcept<K> && std::derived_from<D, T>
        T& emplace_with_allocator(const K& k, std::pmr::memory_resource* resource, Args&&... args) {
            return insert(k, ers::pmr::make_holder<D>(resource, std::forward<Args>(args)...));
        }

    private:
        ers::SwissMap<
            std::string,
            Holder<T>,
            ers::util::string_hash<ers::hashing::Rapid>,
            ers::util::string_equal
        > _content;

        template<typename K>
        T& _add(const K& k, Holder<T> v) {
            auto [it, _] = _content.emplace(k, std::move(v));
            return it->second.get();
        }
    };
}

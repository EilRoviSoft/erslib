#pragma once

// std
#include <any>

// ers
#include <erslib/container/unordered_table.hpp>
#include <erslib/hashing/rapid.hpp>
#include <erslib/memory/pmr/holder.hpp>
#include <erslib/util/string.hpp>

// ecs
#include <easy_ecs/entity.hpp>
#include <easy_ecs/system.hpp>

namespace ecs {
    /*class Engine {
    public:
        template<typename K, typename T>
        T& add(const K& k, T v) {
            auto ptr = ers::pmr::make_polymorphic_holder<IComponent, T>(std::pmr::get_default_resource(), std::move(v));
            auto [it, _] = m_storage.try_emplace(k, std::move(ptr));
            return *it->second;
        }

    protected:
        ers::UnorderedMap<
            std::string,
            ers::pmr::Holder<IComponent>,
            ers::util::string_hash_adaptor<ers::RapidHash>,
            ers::util::string_equal
        > m_storage;
        ers::UnorderedMap<size_t, std::list<IComponent*>> m_by_type;
    };*/
}

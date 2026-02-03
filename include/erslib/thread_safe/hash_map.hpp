#pragma once

// std
#include <mutex>
#include <shared_mutex>

// boost
#include <boost/unordered_map.hpp>

// ers
#include <erslib/type/ref.hpp>

#include "erslib/container/stringly.hpp"

namespace ers::thread_safe {
    template<typename TKey, typename TVal, typename THash, typename TEqual, typename TAlloc>
    class UnorderedMap {
    public:
        using container_type = boost::unordered_map<TKey, TVal, THash, TEqual, TAlloc>;
        using pair_type = std::pair<TKey, TVal>;

        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        UnorderedMap() = default;

        UnorderedMap(const UnorderedMap& another) :
            m_data(another.m_data) {}
        UnorderedMap& operator=(const UnorderedMap& another) {
            std::unique_lock lock(this->m_mutex);
            this->m_data = another.m_data;
            return *this;
        }

        UnorderedMap(UnorderedMap&& another) noexcept :
            m_data(std::move(another.m_data)) {
        }
        UnorderedMap& operator=(UnorderedMap&& another) noexcept {
            std::unique_lock lock(this->m_mutex);
            this->m_data = std::move(another.m_data);
            return *this;
        }

        // capacity

        bool empty() const {
            std::shared_lock lock(this->m_mutex);
            return m_data.empty();
        }

        size_t size() const {
            std::shared_lock lock(this->m_mutex);
            return m_data.size();
        }

        // io

        template<typename T>
        bool set(const T& k, TVal v) {
            std::unique_lock lock(this->m_mutex);

            auto [_, flag] = this->m_data.emplace(k, std::move(v));
            return flag;
        }
        template<typename T>
        optref<const TVal> get(const T& k) const {
            std::shared_lock lock(this->m_mutex);

            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return std::nullopt;
            return it->second;
        }

        // lookup

        template<typename T>
        const TVal& operator[](const T& k) const {
            return *this->get(k);
        }

        const container_type& wrapped_data() const {
            return this->m_data;
        }

    protected:
        mutable std::shared_mutex m_mutex;
        container_type m_data;
    };

    template<typename TVal, typename TAlloc = container::stringly_allocator<TVal>>
    using StringlyMap = UnorderedMap<
        std::string,
        TVal,
        container::stringly_hash,
        container::stringly_equal,
        TAlloc
    >;
}

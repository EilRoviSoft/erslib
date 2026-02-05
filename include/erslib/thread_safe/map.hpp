#pragma once

// std
#include <mutex>
#include <shared_mutex>

// boost
#include <boost/optional.hpp>

// ers
#include <erslib/concept/container.hpp>

namespace ers::thread_safe {
    template<typename K, typename V, typename Container>
    class Map {
    public:
        using container_type = Container;
        using pair_type = std::pair<K, V>;

        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        static_assert(HashMapConcept<container_type>, "Container must satisfy HashMap concept");

        Map() = default;

        Map(const Map& another) :
            m_data(another.m_data) {}
        Map& operator=(const Map& another) {
            std::unique_lock lock(this->m_mutex);
            this->m_data = another.m_data;
            return *this;
        }

        Map(Map&& another) noexcept :
            m_data(std::move(another.m_data)) {
        }
        Map& operator=(Map&& another) noexcept {
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
        bool set(const T& k, V v = V()) {
            std::unique_lock lock(this->m_mutex);

            auto [_, flag] = this->m_data.emplace(k, std::move(v));
            return flag;
        }
        template<typename T>
        boost::optional<const V&> get(const T& k) const {
            std::shared_lock lock(this->m_mutex);

            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return boost::none;
            return it->second;
        }

        // lookup

        template<typename T>
        const V& operator[](const T& k) const {
            return *this->get(k);
        }

        const container_type& wrapped_data() const {
            return this->m_data;
        }

    protected:
        mutable std::shared_mutex m_mutex;
        container_type m_data;
    };
}

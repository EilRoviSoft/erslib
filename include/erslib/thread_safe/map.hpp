#pragma once

// std
#include <mutex>
#include <shared_mutex>

// boost
#include <boost/optional.hpp>

// ers
#include <erslib/concept/container.hpp>


namespace ers::thread_safe {
    template<typename Container>
        requires HashMapConcept<Container>
    class Map {
    public:
        using container_type = Container;
        using key_type = container_type::key_type;
        using mapped_type = container_type::mapped_type;
        using value_type = container_type::value_type;

        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;


        // Constructor

        Map() = default;

        Map(const Map& another) :
            m_data(another.m_data) {
        }
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


        // Destructor

        ~Map() = default;


        // Capacity

        [[nodiscard]]
        bool empty() const {
            std::shared_lock lock(this->m_mutex);
            return m_data.empty();
        }

        [[nodiscard]]
        size_t size() const {
            std::shared_lock lock(this->m_mutex);
            return m_data.size();
        }


        // I/O

        template<typename T>
        bool set(const T& k, mapped_type v = mapped_type()) {
            std::unique_lock lock(this->m_mutex);

            auto [_, flag] = this->m_data.emplace(k, std::move(v));
            return flag;
        }
        template<typename T>
        [[nodiscard]]
        boost::optional<const mapped_type&> get(const T& k) const {
            std::shared_lock lock(this->m_mutex);

            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return boost::none;
            return it->second;
        }


        // Lookup

        template<typename T>
        [[nodiscard]]
        const mapped_type& operator[](const T& k) const {
            return *this->get(k);
        }

        [[nodiscard]]
        const container_type& wrapped_data() const {
            return this->m_data;
        }

    protected:
        mutable std::shared_mutex m_mutex;
        container_type m_data;
    };
}

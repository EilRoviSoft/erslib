#pragma once

// std
#include <concepts>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>

// ers
#include <erslib/core/concept/container.hpp>
#include <erslib/core/type/optional.hpp>


namespace ers::impl::thread_safe {
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


        // Member functions

        Map() = default;

        Map(const Map& another) :
            Map(another, std::shared_lock(another.m_mutex)) {
        }
        Map& operator=(const Map& another) {
            if (this == &another)
                return *this;

            std::unique_lock lhs(this->m_mutex, std::defer_lock);
            std::shared_lock rhs(another.m_mutex, std::defer_lock);
            std::lock(lhs, rhs);

            this->m_data = another.m_data;
            return *this;
        }

        Map(Map&& another) noexcept :
            Map(std::move(another), std::unique_lock(another.m_mutex)) {
        }
        Map& operator=(Map&& another) noexcept {
            if (this == &another)
                return *this;

            std::scoped_lock lock(this->m_mutex, another.m_mutex);

            this->m_data = std::move(another.m_data);
            return *this;
        }

        ~Map() = default;


        // Observers

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


        // Modifiers

        template<typename T>
        bool set(const T& k, mapped_type v = mapped_type()) {
            std::unique_lock lock(this->m_mutex);
            auto [_, flag] = this->m_data.emplace(k, std::move(v));
            return flag;
        }
        template<typename T>
        [[nodiscard]]
        optional<mapped_type> get(const T& k) const requires std::copy_constructible<mapped_type> {
            std::shared_lock lock(this->m_mutex);
            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return nullopt;

            return it->second;
        }

        template<typename T, std::invocable<const mapped_type&> F>
        optional<std::invoke_result_t<F, const mapped_type&>> visit(const T& k, F&& fn) const {
            std::shared_lock lock(this->m_mutex);
            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return nullopt;

            return std::invoke(std::forward<F>(fn), it->second);
        }


        // Accessors

        template<typename T>
        [[nodiscard]]
        mapped_type operator[](const T& k) const requires std::copy_constructible<mapped_type> {
            std::shared_lock lock(this->m_mutex);
            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                throw std::out_of_range("ers::thread_safe::Map: key not found");

            return it->second;
        }

        [[nodiscard]]
        const container_type& wrapped_data() const {
            return this->m_data;
        }


    protected:
        mutable std::shared_mutex m_mutex;
        container_type m_data;


    private:
        Map(const Map& another, std::shared_lock<std::shared_mutex>&&) :
            m_data(another.m_data) {
        }

        Map(Map&& another, std::unique_lock<std::shared_mutex>&&) noexcept :
            m_data(std::move(another.m_data)) {
        }
    };
}


// Exports ---------------------------------------------------------------------

namespace ers::thread_safe {
    using impl::thread_safe::Map;
}

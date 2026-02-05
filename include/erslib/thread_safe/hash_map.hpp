#pragma once

// std
#include <mutex>
#include <shared_mutex>

// boost
#include <boost/unordered_map.hpp>

// ers
#include <erslib/hashing/std.hpp>
#include <erslib/type/ref.hpp>
#include <erslib/util/string.hpp>

namespace ers::thread_safe {
    template<typename K, typename V, typename Hash, typename Equal, typename Alloc>
    class UnorderedMap {
    public:
        using container_type = boost::unordered_map<K, V, Hash, Equal, Alloc>;
        using pair_type = std::pair<K, V>;

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
        bool set(const T& k, V v) {
            std::unique_lock lock(this->m_mutex);

            auto [_, flag] = this->m_data.emplace(k, std::move(v));
            return flag;
        }
        template<typename T>
        optref<const V> get(const T& k) const {
            std::shared_lock lock(this->m_mutex);

            auto it = this->m_data.find(k);

            if (it == this->m_data.end())
                return std::nullopt;
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

    template<
        typename V,
        template<typename> typename HashEngine = hashing::Std,
        typename Alloc = std::allocator<std::pair<std::string, V>>>
    using StringMap = UnorderedMap<
        std::string,
        V,
        util::string_hash<HashEngine>,
        util::string_equal,
        Alloc
    >;
}

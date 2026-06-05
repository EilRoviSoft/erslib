#pragma once

// std
#include <stdexcept>
#include <type_traits>

// ers
#include <erslib/adaptor/transparent/base.hpp>


namespace ers::wrapper {
    template<typename Container>
    class KeyedHashSet {
    public:
        using key_type = size_t;
        using value_type = typename Container::mapped_type;
        using hasher = typename Container::hasher;
        using key_equal = typename Container::key_equal;


    private:
        using wrapped_type = Container;
        template<typename BaseIt>
        struct iterator_adapter {
            BaseIt it;


            using iterator_category = std::forward_iterator_tag;
            using value_type = value_type;
            using difference_type = std::ptrdiff_t;
            using reference = const value_type&;
            using pointer = const value_type*;


            reference operator*() const noexcept { return it->second; }
            pointer operator->() const noexcept { return &it->second; }


            iterator_adapter& operator++() {
                ++it;
                return *this;
            }
            iterator_adapter operator++(int) {
                auto tmp = *this;
                ++it;
                return tmp;
            }


            bool operator==(const iterator_adapter&) const = default;
        };


    public:
        using allocator_type = typename Container::allocator_type;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
        using size_type = typename wrapped_type::size_type;
        using difference_type = typename wrapped_type::difference_type;

        using iterator = iterator_adapter<typename wrapped_type::const_iterator>;
        using const_iterator = iterator;


        // Ctors

        KeyedHashSet() = default;

        explicit KeyedHashSet(const allocator_type& alloc) : m_content(alloc) {}

        explicit KeyedHashSet(
            size_type bucket_count,
            const hasher& hash = {},
            const key_equal& eq = {},
            const allocator_type& alloc = {}
        ) : m_content(bucket_count, hash, eq, alloc) {
        }

        KeyedHashSet(size_type bucket_count, const allocator_type& alloc) :
            m_content(bucket_count, hasher(), key_equal(), alloc) {
        }

        KeyedHashSet(size_type bucket_count, const hasher& hash, const allocator_type& alloc) :
            m_content(bucket_count, hash, key_equal(), alloc) {
        }

        template<typename InputIt>
        KeyedHashSet(
            InputIt first, InputIt last,
            size_type bucket_count = 0,
            const hasher& hash = {},
            const key_equal& eq = {},
            const allocator_type& alloc = {}
        ) :
            m_content(bucket_count, hash, eq, alloc) {
            insert(first, last);
        }

        KeyedHashSet(
            std::initializer_list<value_type> il,
            size_type bucket_count = 0,
            const hasher& hash = {},
            const key_equal& eq = {},
            const allocator_type& alloc = {}
        ) :
            m_content(bucket_count, hash, eq, alloc) {
            insert(il);
        }


        KeyedHashSet& operator=(std::initializer_list<value_type> il) {
            clear();
            insert(il);
            return *this;
        }


        // Copy

        KeyedHashSet(const KeyedHashSet&) = default;

        KeyedHashSet(const KeyedHashSet& other, const allocator_type& alloc) :
            m_content(other.m_content, alloc) {
        }

        KeyedHashSet& operator=(const KeyedHashSet&) = default;


        // Move

        KeyedHashSet(KeyedHashSet&&) noexcept = default;
        
        KeyedHashSet(KeyedHashSet&& other, const allocator_type& alloc) :
            m_content(std::move(other.m_content), alloc) {
        }

        KeyedHashSet& operator=(KeyedHashSet&&) noexcept = default;


        // Dtor

        ~KeyedHashSet() = default;


        // Allocator

        [[nodiscard]] allocator_type get_allocator() const noexcept {
            return m_content.get_allocator();
        }


        // Iterators

        [[nodiscard]] iterator begin() noexcept { return { m_content.begin() }; }
        [[nodiscard]] const_iterator begin() const noexcept { return { m_content.begin() }; }
        [[nodiscard]] const_iterator cbegin() const noexcept { return { m_content.cbegin() }; }

        [[nodiscard]] iterator end() noexcept { return { m_content.end() }; }
        [[nodiscard]] const_iterator end() const noexcept { return { m_content.end() }; }
        [[nodiscard]] const_iterator cend() const noexcept { return { m_content.cend() }; }


        // Capacity

        [[nodiscard]] bool empty() const noexcept { return m_content.empty(); }
        [[nodiscard]] size_type size() const noexcept { return m_content.size(); }
        [[nodiscard]] size_type max_size() const noexcept { return m_content.max_size(); }


        // Modifiers

        void clear() noexcept { m_content.clear(); }


        template<typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args) {
            value_type value(std::forward<Args>(args)...);
            auto key = std::invoke(Member, value);
            auto [it, inserted] = m_content.try_emplace(std::move(key), std::move(value));
            return { iterator { it }, inserted };
        }

        template<typename... Args>
        iterator emplace_hint(const_iterator hint, Args&&... args) {
            value_type value(std::forward<Args>(args)...);
            auto key = std::invoke(Member, value);
            return { m_content.try_emplace(hint.it, std::move(key), std::move(value)) };
        }

        std::pair<iterator, bool> insert(const value_type& v) {
            auto key = std::invoke(Member, v);
            auto [it, inserted] = m_content.try_emplace(std::move(key), v);
            return { iterator { it }, inserted };
        }

        std::pair<iterator, bool> insert(value_type&& v) {
            auto key = std::invoke(Member, v);
            auto [it, inserted] = m_content.try_emplace(std::move(key), std::move(v));
            return { iterator { it }, inserted };
        }

        iterator insert(const_iterator /*hint*/, const value_type& v) { return insert(v).first; }
        iterator insert(const_iterator /*hint*/, value_type&& v) { return insert(std::move(v)).first; }

        template<typename InputIt>
        void insert(InputIt first, InputIt last) {
            for (; first != last; ++first) insert(*first);
        }

        void insert(std::initializer_list<value_type> il) {
            insert(il.begin(), il.end());
        }


        iterator erase(const_iterator pos) { return { m_content.erase(pos.it) }; }
        iterator erase(const_iterator first, const_iterator last) {
            return { m_content.erase(first.it, last.it) };
        }

        template<typename K>
        size_type erase(const K& key) { return m_content.erase(key); }

        void swap(KeyedHashSet& other) noexcept(noexcept(m_content.swap(other.m_content))) {
            m_content.swap(other.m_content);
        }


        // Lookup

        template<typename K>
        [[nodiscard]] iterator find(const K& key) { return { m_content.find(key) }; }
        template<typename K>
        [[nodiscard]] const_iterator find(const K& key) const { return { m_content.find(key) }; }

        template<typename K>
        [[nodiscard]] size_type count(const K& key) const { return m_content.count(key); }

        template<typename K>
        [[nodiscard]] bool contains(const K& key) const { return m_content.contains(key); }

        template<typename K>
        [[nodiscard]] std::pair<iterator, iterator> equal_range(const K& key) {
            auto [a, b] = m_content.equal_range(key);
            return { iterator { a }, iterator { b } };
        }

        template<typename K>
        [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const K& key) const {
            auto [a, b] = m_content.equal_range(key);
            return { const_iterator { a }, const_iterator { b } };
        }


        // I'll skip buckets, because I want this interface to work with any hashing container


        // Hash policy

        [[nodiscard]] float load_factor() const noexcept { return m_content.load_factor(); }
        [[nodiscard]] float max_load_factor() const noexcept { return m_content.max_load_factor(); }
        void max_load_factor(float ml) { m_content.max_load_factor(ml); }

        void rehash(size_type count) { m_content.rehash(count); }
        void reserve(size_type count) { m_content.reserve(count); }


        // Observers

        [[nodiscard]] hasher hash_function() const { return m_content.hash_function(); }
        [[nodiscard]] key_equal key_eq() const { return m_content.key_eq(); }


        template<typename K>
        [[nodiscard]] value_type& at(const K& key) {
            auto it = m_content.find(key);
            if (it == m_content.end())
                throw std::out_of_range("KeyedHashSet::at() — key not found");
            return it->second;
        }
        template<typename K>
        [[nodiscard]] const value_type& at(const K& key) const {
            auto it = m_content.find(key);
            if (it == m_content.end())
                throw std::out_of_range("KeyedHashSet::at() — key not found");
            return it->second;
        }

        // Literally calls this->at(key)
        template<typename K>
        value_type& operator[](const K& key) {
            return this->at(key);
        }
        // Literally calls this->at(key)
        template<typename K>
        value_type& operator[](const K& key) const {
            return this->at(key);
        }


        // Comparison

        [[nodiscard]] friend bool operator==(const KeyedHashSet& a, const KeyedHashSet& b) {
            return a.m_content == b.m_content;
        }


    protected:
        wrapped_type m_content;
    };


    template<auto... Args> // placeholder to allow ADL
    void swap(auto& a, auto& b) noexcept(noexcept(a.swap(b))) requires requires { a.swap(b); } {
        a.swap(b);
    }
}

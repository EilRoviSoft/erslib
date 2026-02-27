#pragma once

// std
#include <ranges>
#include <string_view>
#include <unordered_set>


namespace ers::splitting {
    template<typename T>
    class TIterator;

    template<typename T>
    class Processor : public std::ranges::view_interface<Processor<T>> {
        friend class TIterator<T>;
        friend T;


    public:
        using iterator = T;


    public:
        // Constructor

        explicit Processor(std::string_view content, std::string_view delims = " ") :
            m_storage(content),
            m_delims(delims.begin(), delims.end()) {
        }


        // Iterators

        iterator begin() const {
            return iterator(*this, 0);
        }
        iterator end() const {
            return iterator(*this, m_storage.size());
        }


        // Observers

        std::string_view base() const {
            return m_storage;
        }


    protected:
        std::string_view m_storage;
        std::unordered_set<char> m_delims;
    };

    template<typename T>
    class TIterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;

        using parent_type = Processor<T>;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;


    public:
        // Constructors

        TIterator() = default;
        explicit TIterator(const parent_type& parent, size_t offset) :
            m_parent(&parent),
            m_offset(offset) {
            _interface()._advance();
        }


        // Accessors

        value_type operator*() const {
            if (!m_parent || m_offset >= m_parent->m_storage.size())
                return {};
            return m_parent->m_storage.substr(m_offset, m_length);
        }


        // Modifiers

        T& operator++() {
            if (m_parent && m_offset < m_parent->m_storage.size()) {
                m_offset += m_length;
                _interface()._advance();
            }

            return _interface();
        }
        T operator++(int) {
            auto temp = _interface();
            ++_interface();
            return temp;
        }


        // Comparing

        bool operator==(const TIterator& other) const {
            return m_parent == other.m_parent && m_offset == other.m_offset;
        }


    protected:
        const parent_type* m_parent = nullptr;
        size_t m_offset = 0;
        size_t m_length = 0;


    private:
        T& _interface() { return static_cast<T&>(*this); }
        const T& _interface() const { return static_cast<const T&>(*this); }
    };
}

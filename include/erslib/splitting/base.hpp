#pragma once

// std
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace ers::splitting {
    template<typename T>
    class TIterator;

    template<typename T>
    class Processor : std::ranges::view_interface<Processor<T>> {
        friend class TIterator<T>;
        friend T;

    public:
        using iterator = T;

        explicit Processor(std::string_view content, std::string_view delims = " ") :
            m_content(content),
            m_delims(delims.begin(), delims.end()) {
        }

        iterator begin() const {
            return iterator(*this, 0);
        }
        iterator end() const {
            return iterator(*this, m_content.size());
        }

        std::string_view base() const {
            return m_content;
        }

    protected:
        std::string_view m_content;
        std::unordered_set<char> m_delims;
    };

    template<typename T>
    class TIterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;

        TIterator() = default;
        explicit TIterator(const Processor<T>& parent, size_t offset) :
            m_parent(&parent),
            m_offset(offset) {
            interface()->_advance();
        }

        value_type operator*() const {
            if (!m_parent || m_offset >= m_parent->m_content.size())
                return {};
            return m_parent->m_content.substr(m_offset, m_length);
        }

        T& operator++() {
            if (m_parent && m_offset < m_parent->m_content.size()) {
                m_offset += m_length;
                interface()->_advance();
            }

            return *interface();
        }
        T operator++(int) {
            auto temp = *interface();
            ++*interface();
            return temp;
        }

        bool operator==(const TIterator& other) const {
            return m_parent == other.m_parent && m_offset == other.m_offset;
        }

    protected:
        const Processor<T>* m_parent = nullptr;
        size_t m_offset = 0;
        size_t m_length = 0;

    private:
        T* interface() { return static_cast<T*>(this); }
        const T* interface() const { return static_cast<const T*>(this); }
    };
}

#pragma once

// std
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace ers::splitting {
    class Smart : public std::ranges::view_interface<Smart> {
        friend class SmartIterator;

        using iterator = SmartIterator;

    public:
        explicit Smart(std::string_view content, std::string_view delims = " ");

        iterator begin() const;
        iterator end() const;

        std::string_view base() const;

    private:
        std::string_view _content;
        std::unordered_set<char> _delims;
    };
}

namespace ers::splitting {
    class SmartIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;

        explicit SmartIterator(const Smart& parent, std::size_t offset = 0);

        value_type operator*() const;

        SmartIterator& operator++();
        SmartIterator operator++(int);

        bool operator==(const SmartIterator& other) const;

    private:
        const Smart* _parent = nullptr;
        size_t _offset = 0;
        size_t _length = 0;
        bool _in_quote = false;

        void _advance();
    };
}

#pragma once

// std
#include <ranges>
#include <string>
#include <unordered_set>

namespace ers::splitting {
    class Regular : public std::ranges::view_interface<Regular> {
        friend class RegularIterator;

        using iterator = RegularIterator;

    public:
        explicit Regular(std::string_view content, std::string_view delims = " ");

        iterator begin() const;
        iterator end() const;

        std::string_view base() const;

    private:
        std::string_view _content;
        std::unordered_set<char> _delims;
    };
}

namespace ers::splitting {
    class RegularIterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;

        RegularIterator() = default;
        explicit RegularIterator(const Regular& parent, size_t offset = 0);

        value_type operator*() const;

        RegularIterator& operator++();
        RegularIterator operator++(int);

        bool operator==(const RegularIterator& other) const;

    private:
        const Regular* _parent = nullptr;
        size_t _offset = 0;
        size_t _length = 0;

        void _advance();
    };
}

namespace ers {
    using RegularSplitter = splitting::Regular;
}

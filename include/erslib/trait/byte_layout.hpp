#pragma once

// std
#include <concepts>
#include <cstring>

// ers
#include <erslib/concept/container.hpp>

#include "byte_layout.hpp"

// declaration

namespace ers {
    template<typename T>
    struct byte_layout_traits {
        static constexpr const void* data(const T& v) noexcept = delete("non-specialized definition");

        static constexpr size_t size(const T&) noexcept = delete("non-specialized definition");
    };
}

// specializations

template<std::integral T>
struct ers::byte_layout_traits<T> {
    static constexpr const void* data(const T& v) noexcept {
        return &v;
    }

    static constexpr size_t size(const T&) noexcept {
        return sizeof(T);
    }
};

template<typename T>
    requires ers::ContiguousContainer<T>
struct ers::byte_layout_traits<T> {
    static constexpr const void* data(const T& v) noexcept {
        return v.data();
    }

    static constexpr size_t size(const T& v) noexcept {
        return v.size() * sizeof(typename T::value_type);
    }
};

template<>
struct ers::byte_layout_traits<const char*> {
    static constexpr const void* data(const char* v) noexcept {
        return v;
    }

    static constexpr size_t size(const char* v) noexcept {
        return strlen(v);
    }
};

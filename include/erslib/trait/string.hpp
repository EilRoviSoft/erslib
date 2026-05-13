#pragma once

// std
#include <string>


// Declaration

namespace ers {
    template<typename T>
    struct string_traits_impl {
        static constexpr size_t size(const T& arg) = delete("non-specialized template");
        static void append(std::string& dest, T&& source) = delete("non-specialized template");
        static constexpr char index(const T& source, size_t i) = delete("non-specialized template");
    };

    template<typename T>
    struct string_traits : string_traits_impl<std::remove_cvref_t<T>> {};
}


// Specializations

template<>
struct ers::string_traits_impl<char> {
    static constexpr bool is_comptime = true;

    static constexpr size_t size(char) {
        return 1;
    }

    static void append(std::string& dest, const char source) {
        dest += source;
    }

    static constexpr char index(char source, [[maybe_unused]] size_t i) {
        return source;
    }
};

template<>
struct ers::string_traits_impl<std::string> {
    static constexpr bool is_comptime = false;

    static constexpr size_t size(const std::string& arg) {
        return arg.size();
    }

    static void append(std::string& dest, const std::string& source) {
        dest += source;
    }

    static constexpr char index(const std::string& source, size_t i) {
        return source[i];
    }
};

template<>
struct ers::string_traits_impl<std::string_view> {
    static constexpr bool is_comptime = true;

    static constexpr size_t size(std::string_view arg) {
        return arg.size();
    }

    static void append(std::string& dest, std::string_view source) {
        dest += source;
    }

    static constexpr char index(std::string_view source, size_t i) {
        return source[i];
    }
};

template<>
struct ers::string_traits_impl<const char*> {
    static constexpr bool is_comptime = true;

    static constexpr size_t size(const char* arg) {
        return arg ? std::char_traits<char>::length(arg) : 0;
    }

    static void append(std::string& dest, const char* source) {
        if (source)
            dest += source;
    }

    static constexpr char index(const char* source, size_t i) {
        return source[i];
    }
};

template<size_t N>
struct ers::string_traits_impl<const char(&)[N]> {
    static constexpr bool is_comptime = true;

    static constexpr size_t size(const char (&)[N]) {
        return N;
    }

    static void append(std::string& dest, const char (&source)[N]) {
        dest += source;
    }

    static constexpr char index(const char (&source)[N], size_t i) {
        return source[i];
    }
};

#pragma once

// std
#include <string>

namespace ers {
    template<typename T>
    struct stringly_traits {
        static constexpr size_t size(const T& arg) = delete("non-specialized definition");

        static void append(std::string& dest, T&& source) = delete("non-specialized definition");
    };

    template<>
    struct stringly_traits<char> {
        static constexpr size_t size(char) {
            return 1;
        }

        static void append(std::string& dest, const char source) {
            dest += source;
        }
    };

    template<>
    struct stringly_traits<std::string> {
        static constexpr size_t size(const std::string& arg) {
            return arg.size();
        }

        static void append(std::string& dest, const std::string& source) {
            dest += source;
        }
    };

    template<>
    struct stringly_traits<std::string_view> {
        static constexpr size_t size(const std::string_view& arg) {
            return arg.size();
        }

        static void append(std::string& dest, const std::string_view& source) {
            dest += source;
        }
    };

    template<>
    struct stringly_traits<const char*> {
        static constexpr size_t size(const char* arg) {
            return arg ? std::strlen(arg) : 0;
        }

        static void append(std::string& dest, const char* source) {
            if (source)
                dest += source;
        }
    };

    template<size_t TSize>
    struct stringly_traits<const char[TSize]> {
        static constexpr size_t size(const char (&arg)[TSize]) {
            return std::strlen(arg);
        }

        static void append(std::string& dest, const char (&source)[TSize]) {
            dest += source;
        }
    };
}

namespace ers::trait {
    template<typename T>
    constexpr size_t get_size(T&& arg) {
        using decay_type = std::decay_t<T>;
        return stringly_traits<decay_type>::size(std::forward<T>(arg));
    }

    template<typename T>
    void append_to_string(std::string& dest, T&& source) {
        using decay_type = std::decay_t<T>;
        stringly_traits<decay_type>::append(dest, std::forward<T>(source));
    }
}

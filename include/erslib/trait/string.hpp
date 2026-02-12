#pragma once

// std
#include <string>

namespace ers {
    template<typename T>
    struct string_traits {
        static constexpr size_t size(const T& arg) = delete("non-specialized definition");

        static void append(std::string& dest, T&& source) = delete("non-specialized definition");
    };

    template<>
    struct string_traits<char> {
        static constexpr bool is_comptime = true;

        static constexpr size_t size(char) {
            return 1;
        }

        static void append(std::string& dest, const char source) {
            dest += source;
        }
    };

    template<>
    struct string_traits<std::string> {
        static constexpr bool is_comptime = false;

        static constexpr size_t size(const std::string& arg) {
            return arg.size();
        }

        static void append(std::string& dest, const std::string& source) {
            dest += source;
        }
    };

    template<>
    struct string_traits<std::string_view> {
        static constexpr bool is_comptime = true;

        static constexpr size_t size(const std::string_view& arg) {
            return arg.size();
        }

        static void append(std::string& dest, const std::string_view& source) {
            dest += source;
        }
    };

    template<>
    struct string_traits<const char*> {
        static constexpr bool is_comptime = true;

        static constexpr size_t size(const char* arg) {
            return arg ? std::char_traits<char>::length(arg) : 0;
        }

        static void append(std::string& dest, const char* source) {
            if (source)
                dest += source;
        }
    };

    template<size_t TSize>
    struct string_traits<const char[TSize]> {
        static constexpr bool is_comptime = true;

        static constexpr size_t size(const char (&arg)[TSize]) {
            return std::strlen(arg);
        }

        static void append(std::string& dest, const char (&source)[TSize]) {
            dest += source;
        }
    };
}

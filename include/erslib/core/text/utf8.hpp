#pragma once

// std
#include <string>
#include <string_view>


namespace ers::impl::utf8 {
    constexpr bool is_continuation(char c) noexcept {
        return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
    }

    constexpr std::string_view truncate(std::string_view text, size_t max_bytes) noexcept {
        if (text.size() <= max_bytes)
            return text;


        size_t end = max_bytes;

        while (end > 0 && is_continuation(text[end]))
            --end;

        return text.substr(0, end);
    }

    constexpr bool is_valid(std::string_view text) noexcept {
        size_t i = 0;

        while (i < text.size()) {
            auto c = static_cast<unsigned char>(text[i]);
            size_t length;
            char32_t code;

            if (c < 0x80) {
                ++i;
                continue;
            } else if ((c & 0xE0) == 0xC0) {
                length = 2;
                code = c & 0x1F;
            } else if ((c & 0xF0) == 0xE0) {
                length = 3;
                code = c & 0x0F;
            } else if ((c & 0xF8) == 0xF0) {
                length = 4;
                code = c & 0x07;
            } else
                return false;

            if (i + length > text.size())
                return false;


            for (size_t j = 1; j < length; ++j) {
                if (!is_continuation(text[i + j]))
                    return false;

                code = (code << 6) | (static_cast<unsigned char>(text[i + j]) & 0x3F);
            }


            constexpr char32_t min_code[] = { 0, 0, 0x80, 0x800, 0x10000 };

            if (code < min_code[length] || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF))
                return false;


            i += length;
        }

        return true;
    }

    constexpr void append(std::string& out, char32_t code) {
        if (code < 0x80) {
            out += static_cast<char>(code);
        } else if (code < 0x800) {
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            out += static_cast<char>(0xE0 | (code >> 12));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (code >> 18));
            out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        }
    }
}


// Exports ---------------------------------------------------------------------

namespace ers::utf8 {
    using impl::utf8::truncate;
    using impl::utf8::is_valid;
    using impl::utf8::append;
}

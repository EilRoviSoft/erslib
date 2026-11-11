#include "erslib/core/text/language_layout.hpp"

// std
#include <array>
#include <span>
#include <vector>

// ers
#include <erslib/core/text/utf8.hpp>


// Details ---------------------------------------------------------------------

namespace {
    struct layout_key_t {
        char en;
        char32_t ru;
    };

    constexpr unsigned char layout_file[] = {
        #embed "layout_ru.txt"
    };

    consteval char32_t next_code(std::span<const unsigned char> bytes, std::size_t& i) {
        const unsigned char lead = bytes[i++];

        if (lead < 0x80)
            return lead;

        const std::size_t length = lead >= 0xF0 ? 4 : lead >= 0xE0 ? 3 : 2;
        char32_t code = lead & (0x7F >> length);

        for (std::size_t k = 1; k < length; ++k)
            code = (code << 6) | (bytes[i++] & 0x3F);

        return code;
    }

    consteval std::vector<std::u32string> read_lines(std::span<const unsigned char> bytes) {
        std::vector<std::u32string> out(1);
        std::size_t i = 0;

        while (i < bytes.size()) {
            const char32_t code = next_code(bytes, i);

            if (code == U'\n')
                out.emplace_back();
            else if (code != U'\r' && code != 0xFEFF)
                out.back() += code;
        }

        std::erase_if(out, [](const auto& line) { return line.empty(); });
        return out;
    }


    constexpr auto en_to_ru = [] {
        std::array<char32_t, 128> out {};
        const auto lines = read_lines(layout_file);

        if (lines.size() % 2 != 0)
            throw "layout_ru.txt: a latin line without its russian line";

        for (std::size_t row = 0; row < lines.size(); row += 2) {
            const auto& en = lines[row];
            const auto& ru = lines[row + 1];

            if (en.size() != ru.size())
                throw "layout_ru.txt: lines of a pair differ in length";

            for (std::size_t k = 0; k < en.size(); ++k) {
                if (en[k] >= out.size() || out[en[k]] != 0)
                    throw "layout_ru.txt: a key is not ASCII or is repeated";

                out[en[k]] = ru[k];
            }
        }

        return out;
    }();
}


// Implementation --------------------------------------------------------------

std::string ers::impl::text::ru_to_en(std::string_view text) {
    std::string out;
    out.reserve(text.size() * 2);

    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);

        if (byte < en_to_ru.size() && en_to_ru[byte] != 0)
            utf8::append(out, en_to_ru[byte]);
        else
            out += c;
    }

    return out;
}

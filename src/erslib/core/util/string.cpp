#include "erslib/core/util/string.hpp"

// ers
#include <erslib/core/type/general.hpp>


// replace

std::string ers::impl::util::replace(std::string_view where, std::string_view from, std::string_view to, size_t estimated_replacements) {
    std::string result;


    if (where.empty())
        return result;


    const size_t estimated_buffer_size = static_cast<ptrdiff_t>(where.size())
        + (static_cast<ptrdiff_t>(to.size()) - static_cast<ptrdiff_t>(from.size()))
        * estimated_replacements;

    result.reserve(estimated_buffer_size);


    size_t prev = 0, curr;

    while ((curr = where.find(from, prev)) != std::string_view::npos) {
        result += where.substr(prev, curr - prev);
        result += to;
        prev = curr + from.size();
    }


    result += where.substr(prev);

    return result;
}


// quote

std::string ers::impl::util::quote(std::string_view what) {
    std::string out;
    out.reserve(what.size() + 2);

    out += '\'';

    for (const char c : what) {
        if (c == '\'' || c == '\\')
            out += '\\';

        out += c;
    }

    out += '\'';
    return out;
}


// bytes_to_string

namespace {
    enum class EMode : u8 { HexLower, HexUpper, Text };

    constexpr std::string_view lower_digits = "0123456789abcdef";
    constexpr std::string_view upper_digits = "0123456789ABCDEF";
}

std::string ers::impl::util::bytes_to_string(std::span<const std::byte> bytes, std::string_view args) {
    auto it = args.begin();
    auto mode = EMode::HexLower;

    if (it != args.end()) {
        switch (*it) {
            case 'x': mode = EMode::HexLower;
                ++it;
                break;

            case 'X': mode = EMode::HexUpper;
                ++it;
                break;

            case 's': mode = EMode::Text;
                ++it;
                break;

            default:
                break;
        }
    }

    std::string out;

    if (mode != EMode::Text) {
        const std::string_view digits = mode == EMode::HexLower ? lower_digits : upper_digits;
        out.reserve(bytes.size() * 2);

        for (auto byte : bytes) {
            auto v = std::to_integer<unsigned char>(byte);
            out.push_back(digits[v >> 4]);
            out.push_back(digits[v & 0xF]);
        }
    } else
        out.assign(reinterpret_cast<char const*>(bytes.data()), bytes.size());

    return out;
}

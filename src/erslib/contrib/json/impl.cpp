#include "erslib/contrib/json/impl.hpp"


// Helpers ---------------------------------------------------------------------

void utl::impl::json_detail::append_utf8(std::string& out, u32 codepoint) {
    if (codepoint <= 0x7F) {
        out += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        out += static_cast<char>(0xC0 | (codepoint >> 6));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (codepoint >> 12));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (codepoint >> 18));
        out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

void utl::impl::json_detail::append_escaped(std::string& out, std::string_view value) {
    out += '"';

    std::size_t segment = 0;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const auto c = static_cast<unsigned char>(value[i]);
        if (c >= 0x20 && c != '"' && c != '\\')
            continue;

        out += value.substr(segment, i - segment);
        segment = i + 1;

        switch (c) {
            case '"': out += "\\\"";
                break;
            case '\\': out += "\\\\";
                break;
            case '\b': out += "\\b";
                break;
            case '\f': out += "\\f";
                break;
            case '\n': out += "\\n";
                break;
            case '\r': out += "\\r";
                break;
            case '\t': out += "\\t";
                break;
            default:
                std::format_to(std::back_inserter(out), "\\u{:04x}", static_cast<unsigned>(c));
        }
    }

    out += value.substr(segment);
    out += '"';
}

std::string utl::impl::json_detail::pretty_error(std::size_t cursor, std::string_view chars) {
    if (chars.empty())
        return {};

    if (cursor >= chars.size())
        cursor = chars.size() - 1;

    std::size_t line_number = 1;
    for (std::size_t pos = 0; pos < cursor; ++pos) {
        if (chars[pos] == '\n')
            ++line_number;
    }

    constexpr std::size_t max_left_width = 24;
    constexpr std::size_t max_right_width = 24;

    std::size_t line_start = cursor;
    while (line_start > 0 && chars[line_start - 1] != '\n' && cursor - line_start < max_left_width)
        --line_start;

    std::size_t line_end = cursor;
    while (line_end < chars.size() - 1 && chars[line_end + 1] != '\n' && line_end - cursor < max_right_width)
        ++line_end;

    std::string line(chars.substr(line_start, line_end - line_start + 1));
    for (auto& c : line) {
        if (static_cast<unsigned char>(c) > 127)
            c = '?';
    }

    const auto prefix = std::format("Line {}: ", line_number);

    std::string result;
    result += '\n';
    result += prefix;
    result += line;
    result += '\n';
    result.append(prefix.size(), ' ');
    result.append(cursor - line_start, '-');
    result += '^';
    result.append(line_end - cursor, '-');
    result += " [!]";

    return result;
}

std::string utl::impl::json_detail::read_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw ers::make_path_error("Could not open file '{}'", path.string());

    const auto size = file.tellg();
    if (size < 0)
        throw ers::make_path_error("Could not determine size of file '{}'", path.string());

    std::string chars(static_cast<std::size_t>(size), '\0');

    file.seekg(0);
    file.read(chars.data(), size);

    if (!file)
        throw ers::make_path_error("Could not read file '{}'", path.string());

    return chars;
}


// Instantiations --------------------------------------------------------------

template class utl::impl::TNode<utl::impl::default_traits>;

template utl::impl::Node utl::impl::from_string<utl::impl::Node>(std::string_view, std::size_t);
template utl::impl::Node utl::impl::from_file<utl::impl::Node>(const std::filesystem::path&, std::size_t);
template ers::Result<utl::impl::Node> utl::impl::try_from_string<utl::impl::Node>(std::string_view, std::size_t);
template ers::Result<utl::impl::Node> utl::impl::try_from_file<utl::impl::Node>(const std::filesystem::path&, std::size_t);

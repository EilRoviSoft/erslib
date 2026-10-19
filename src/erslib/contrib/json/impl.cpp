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

    std::string chars(size, '\0');

    file.seekg(0);
    file.read(chars.data(), size);

    if (!file)
        throw ers::make_path_error("Could not read file '{}'", path.string());

    return chars;
}


// Pointer ---------------------------------------------------------------------

utl::impl::JsonPointer::JsonPointer(std::string_view pointer) {
    if (pointer.empty())
        return;

    if (pointer.front() != '/')
        throw ers::make_parse_error("JSON pointer '{}' must start with '/'", pointer);

    std::string token;
    for (std::size_t i = 1; i < pointer.size(); ++i) {
        const char c = pointer[i];

        if (c == '/') {
            _tokens.push_back(std::move(token));
            token.clear();
            continue;
        }

        if (c != '~') {
            token += c;
            continue;
        }

        const char next = i + 1 < pointer.size() ? pointer[i + 1] : '\0';
        if (next != '0' && next != '1') {
            throw ers::make_parse_error("JSON pointer '{}' has an invalid escape at position {}",
                pointer, i);
        }

        token += next == '0' ? '~' : '/';
        ++i;
    }

    _tokens.push_back(std::move(token));
}


std::optional<std::size_t> utl::impl::JsonPointer::array_index(std::string_view token) noexcept {
    if (token.empty() || (token.size() > 1 && token.front() == '0'))
        return std::nullopt;

    std::size_t index {};
    const auto* last = token.data() + token.size();
    if (auto r = std::from_chars(token.data(), last, index); !r || r.ptr != last)
        return std::nullopt;

    return index;
}


std::string utl::impl::JsonPointer::to_string() const {
    std::string result;

    for (const auto& token : _tokens) {
        result += '/';

        for (const char c : token) {
            if (c == '~')
                result += "~0";
            else if (c == '/')
                result += "~1";
            else
                result += c;
        }
    }

    return result;
}


// Literals --------------------------------------------------------------------


utl::impl::Json utl::impl::literals::operator""_json(const char* chars, std::size_t size) {
    return from_string(std::string_view(chars, size));
}


// Instantiations --------------------------------------------------------------

template class utl::impl::TNode<utl::impl::default_traits>;
template class utl::impl::TNode<utl::impl::ordered_traits>;

template utl::impl::Json utl::impl::from_string<utl::impl::Json>(std::string_view, std::size_t);
template utl::impl::Json utl::impl::from_file<utl::impl::Json>(const std::filesystem::path&, std::size_t);
template ers::Result<utl::impl::Json> utl::impl::try_from_string<utl::impl::Json>(std::string_view, std::size_t);
template ers::Result<utl::impl::Json> utl::impl::try_from_file<utl::impl::Json>(const std::filesystem::path&, std::size_t);

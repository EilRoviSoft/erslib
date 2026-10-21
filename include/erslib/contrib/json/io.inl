#pragma once

// std
#include <array>
#include <charconv>
#include <cmath>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>


// Helpers ---------------------------------------------------------------------

namespace utl::impl::json_detail {
    constexpr std::size_t default_recursion_limit = 1000;


    void ERSLIB_EXPORT append_utf8(std::string& out, u32 codepoint);
    void ERSLIB_EXPORT append_escaped(std::string& out, std::string_view value);
    std::string ERSLIB_EXPORT pretty_error(std::size_t cursor, std::string_view chars);
    std::string ERSLIB_EXPORT read_file(const std::filesystem::path& path);


    template<class String>
    void append_string(std::string& out, const String& value) {
        const auto& utf8 = string_adaptor<String>::to_utf8(value);
        append_escaped(out, std::string_view(utf8));
    }

    template<std::integral Integral>
    void append_integral(std::string& out, Integral value) {
        std::array<char, std::numeric_limits<Integral>::digits10 + 3> buffer;
        auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
        out.append(buffer.data(), result.ptr);
    }

    template<std::floating_point Floating>
    void append_floating(std::string& out, Floating value) {
        if (!std::isfinite(value)) {
            out += "null";
            return;
        }

        std::array<char, 64> buffer;
        auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
        if (result.ec != std::errc {})
            throw ers::make_format_error("Can't format floating value {}", value);

        std::string_view text(buffer.data(), result.ptr);
        out += text;

        if (text.find_first_of(".eE") == std::string_view::npos)
            out += ".0";
    }
}


// Parser ----------------------------------------------------------------------

namespace utl::impl::json_detail {
    template<class Node>
    class Parser {
        using object_type = typename Node::object_type;
        using array_type = typename Node::array_type;
        using string_type = typename Node::string_type;
        using key_type = typename object_type::key_type;
        using integral_type = typename Node::integral_type;
        using floating_type = typename Node::floating_type;


    public:
        Parser(std::string_view chars, std::size_t recursion_limit) noexcept :
            _chars(chars),
            _recursion_limit(recursion_limit) {
        }


        [[nodiscard]] Node parse() {
            _skip_whitespace();
            auto root = _parse_value();

            _skip_whitespace();
            if (_cursor != _chars.size()) {
                _fail("Unexpected character '{}' after the JSON value",
                    _chars[_cursor]);
            }

            return root;
        }


    private:
        std::string_view _chars;
        std::size_t _recursion_limit;
        std::size_t _cursor = 0;
        std::size_t _depth = 0;


        // -- Errors --
        // ------------

        template<class... Args>
        [[noreturn]] void _fail(std::format_string<Args...> format, Args&&... args) const {
            throw ers::make_parse_error("{} (pos {}){}",
                std::format(format, std::forward<Args>(args)...), _cursor, pretty_error(_cursor, _chars));
        }


        // -- Cursor --
        // ------------

        [[nodiscard]]
        char _peek() const {
            if (_cursor >= _chars.size())
                _fail("Unexpected end of JSON");
            return _chars[_cursor];
        }

        [[nodiscard]]
        bool _at(char c) const noexcept {
            return _cursor < _chars.size() && _chars[_cursor] == c;
        }

        [[nodiscard]]
        bool _at_digit() const noexcept {
            return _cursor < _chars.size() && _chars[_cursor] >= '0' && _chars[_cursor] <= '9';
        }

        void _skip_whitespace() noexcept {
            while (_cursor < _chars.size()) {
                const char c = _chars[_cursor];
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                    return;
                ++_cursor;
            }
        }

        void _skip_digits() noexcept {
            while (_at_digit())
                ++_cursor;
        }

        void _require_digits() {
            if (!_at_digit())
                _fail("Expected a digit");
            _skip_digits();
        }

        void _enter() {
            if (++_depth > _recursion_limit) {
                _fail("JSON nesting is deeper than the limit of {}",
                    _recursion_limit);
            }
        }

        void leave() noexcept {
            --_depth;
        }


        // -- Values --
        // ------------

        [[nodiscard]] Node _parse_value() {
            char c = _peek();

            switch (c) {
                case '{':
                    return Node(_parse_object());

                case '[':
                    return Node(_parse_array());

                case '"':
                    return Node(string_adaptor<string_type>::from_utf8(_parse_raw_string()));

                case 't':
                    _expect_literal("true");
                    return Node(true);

                case 'f':
                    _expect_literal("false");
                    return Node(false);

                case 'n':
                    _expect_literal("null");
                    return Node(nullptr);

                default:
                    if (c == '-' || _at_digit())
                        return _parse_number();
                    _fail("Unexpected character '{}', expected a JSON value", c);
            }
        }

        [[nodiscard]] object_type _parse_object() {
            _enter();
            ++_cursor;

            object_type object;

            _skip_whitespace();
            if (_at('}')) {
                ++_cursor;
                leave();
                return object;
            }

            while (true) {
                _skip_whitespace();
                if (_peek() != '"')
                    _fail("Expected '\"' at the start of an object key");

                auto key = string_adaptor<key_type>::from_utf8(_parse_raw_string());

                _skip_whitespace();
                if (_peek() != ':')
                    _fail("Expected ':' after an object key");
                ++_cursor;

                _skip_whitespace();
                object.try_emplace(std::move(key), _parse_value());

                _skip_whitespace();
                if (_at('}')) {
                    ++_cursor;
                    break;
                }
                if (_peek() != ',')
                    _fail("Expected ',' or '}}' after an object value");
                ++_cursor;
            }

            leave();
            return object;
        }

        [[nodiscard]] array_type _parse_array() {
            _enter();
            ++_cursor;

            array_type array;

            _skip_whitespace();
            if (_at(']')) {
                ++_cursor;
                leave();
                return array;
            }

            while (true) {
                _skip_whitespace();
                array.emplace_back(_parse_value());

                _skip_whitespace();
                if (_at(']')) {
                    ++_cursor;
                    break;
                }
                if (_peek() != ',')
                    _fail("Expected ',' or ']' after an array element");
                ++_cursor;
            }

            leave();
            return array;
        }

        void _expect_literal(std::string_view literal) {
            if (_chars.substr(_cursor, literal.size()) != literal)
                _fail("Expected '{}'", literal);
            _cursor += literal.size();
        }


        // -- Numbers --
        // -------------

        [[nodiscard]] Node _parse_number() {
            std::size_t start = _cursor;
            bool floating = false;

            if (_at('-'))
                ++_cursor;

            if (_at('0'))
                ++_cursor;
            else
                _require_digits();

            if (_at('.')) {
                floating = true;
                ++_cursor;
                _require_digits();
            }

            if (_at('e') || _at('E')) {
                floating = true;
                ++_cursor;
                if (_at('+') || _at('-'))
                    ++_cursor;
                _require_digits();
            }


            auto token = _chars.substr(start, _cursor - start);
            auto* first = token.data();
            auto* last = token.data() + token.size();

            if (floating) {
                floating_type value{};
                if (!std::from_chars(first, last, value)) {
                    _cursor = start;
                    _fail("Number {} doesn't fit into the JSON floating type", token);
                }

                return Node(value);
            } else {
                integral_type value{};
                if (!std::from_chars(first, last, value)) {
                    _cursor = start;
                    _fail("Integer {} doesn't fit into the JSON integral type", token);
                }

                return Node(value);
            }
        }


        // -- Strings --
        // -------------

        [[nodiscard]] std::string _parse_raw_string() {
            ++_cursor;

            std::string result;
            std::size_t segment = _cursor;

            while (_cursor < _chars.size()) {
                char c = _chars[_cursor];

                if (c == '"') {
                    result += _chars.substr(segment, _cursor - segment);
                    ++_cursor;
                    return result;
                }

                if (c == '\\') {
                    result += _chars.substr(segment, _cursor - segment);
                    ++_cursor;
                    _parse_escape(result);
                    segment = _cursor;
                    continue;
                }

                if (static_cast<unsigned char>(c) < 0x20) {
                    _fail("Unescaped control character U+{:04X} in a string",
                        static_cast<unsigned>(static_cast<unsigned char>(c)));
                }

                ++_cursor;
            }

            _fail("Unterminated string");
        }

        void _parse_escape(std::string& out) {
            char c = _peek();
            ++_cursor;

            switch (c) {
                case '"':
                    out += '"';
                    return;

                case '\\':
                    out += '\\';
                    return;

                case '/':
                    out += '/';
                    return;

                case 'b':
                    out += '\b';
                    return;

                case 'f':
                    out += '\f';
                    return;

                case 'n':
                    out += '\n';
                    return;

                case 'r':
                    out += '\r';
                    return;

                case 't':
                    out += '\t';
                    return;

                case 'u':
                    _parse_unicode_escape(out);
                    return;

                default:
                    --_cursor;
                    _fail("Unknown escape sequence '\\{}'", c);
            }
        }

        [[nodiscard]] u16 _parse_hex4() {
            if (_chars.size() - _cursor < 4)
                _fail("Unexpected end of JSON inside a \\u escape");

            auto* first = _chars.data() + _cursor;
            u16 value {};
            if (auto r = std::from_chars(first, first + 4, value, 16); !r || r.ptr != first + 4)
                _fail("Invalid \\u escape '{}'", std::string_view(first, 4));

            _cursor += 4;
            return value;
        }

        void _parse_unicode_escape(std::string& out) {
            u16 high = _parse_hex4();
            u32 codepoint = high;

            if (high >= 0xD800 && high <= 0xDBFF) {
                if (_chars.substr(_cursor, 2) != "\\u")
                    _fail("High surrogate \\u{:04X} isn't followed by a low surrogate", high);
                _cursor += 2;

                u16 low = _parse_hex4();
                if (low < 0xDC00 || low > 0xDFFF)
                    _fail("\\u{:04X} isn't a low surrogate", low);

                codepoint = 0x10000 + ((static_cast<u32>(high) - 0xD800) << 10) + (static_cast<u32>(low) - 0xDC00);
            } else if (high >= 0xDC00 && high <= 0xDFFF) {
                _fail("Lone low surrogate \\u{:04X}", high);
            }

            append_utf8(out, codepoint);
        }
    };
}


// Serializer ------------------------------------------------------------------

namespace utl::impl::json_detail {
    template<bool Pretty, class Node>
    void serialize(std::string& out, const Node& node, std::size_t depth) {
        constexpr std::size_t indent_width = 4;

        auto newline = [&](std::size_t level) {
            if constexpr (Pretty) {
                out += '\n';
                out.append(level * indent_width, ' ');
            }
        };

        switch (node.type()) {
            case ENodeType::None:
                out += "null";
                return;

            case ENodeType::Bool:
                out += node.as_bool() ? "true" : "false";
                return;

            case ENodeType::Integral:
                append_integral(out, node.as_integral());
                return;

            case ENodeType::Floating:
                append_floating(out, node.as_floating());
                return;

            case ENodeType::String:
                append_string(out, node.as_string());
                return;

            case ENodeType::Array: {
                const auto& array = node.as_array();
                if (array.empty()) {
                    out += "[]";
                    return;
                }

                out += '[';
                bool first = true;
                for (const auto& item : array) {
                    if (!first)
                        out += ',';
                    first = false;

                    newline(depth + 1);
                    serialize<Pretty>(out, item, depth + 1);
                }
                newline(depth);
                out += ']';
                return;
            }

            case ENodeType::Object: {
                const auto& object = node.as_object();
                if (object.empty()) {
                    out += "{}";
                    return;
                }

                out += '{';
                bool first = true;
                for (const auto& [key, value] : object) {
                    if (!first)
                        out += ',';
                    first = false;

                    newline(depth + 1);
                    append_string(out, key);
                    out += Pretty ? ": " : ":";
                    serialize<Pretty>(out, value, depth + 1);
                }
                newline(depth);
                out += '}';
                return;
            }
        }
    }
}


// Node serialization ----------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    std::string TNode<Traits>::to_string(EFormat format) const {
        std::string out;

        if (format == EFormat::Pretty)
            json_detail::serialize<true>(out, *this, 0);
        else
            json_detail::serialize<false>(out, *this, 0);

        return out;
    }

    template<JsonTraits Traits>
    void TNode<Traits>::to_file(const std::filesystem::path& path, EFormat format) const {
        auto chars = to_string(format);

        if (path.has_parent_path())
            std::filesystem::create_directories(path.parent_path());

        std::ofstream file(path, std::ios::binary);
        file.write(chars.data(), static_cast<std::streamsize>(chars.size()));

        if (!file)
            throw ers::make_path_error("Could not write JSON to '{}'", path.string());
    }
}


// Parsing API -----------------------------------------------------------------

namespace utl::impl {
    template<class Node = TNode<default_traits>>
        requires json_detail::is_node_v<Node>
    [[nodiscard]]
    Node from_string(std::string_view chars, std::size_t recursion_limit = json_detail::default_recursion_limit) {
        return json_detail::Parser<Node>(chars, recursion_limit).parse();
    }

    template<class Node = TNode<default_traits>>
        requires json_detail::is_node_v<Node>
    [[nodiscard]]
    Node from_file(const std::filesystem::path& path, std::size_t recursion_limit = json_detail::default_recursion_limit) {
        auto chars = json_detail::read_file(path);
        return from_string<Node>(chars, recursion_limit);
    }

    template<class Node = TNode<default_traits>>
        requires json_detail::is_node_v<Node>
    [[nodiscard]]
    ers::Result<Node> try_from_string(std::string_view chars, std::size_t recursion_limit = json_detail::default_recursion_limit) try {
        return from_string<Node>(chars, recursion_limit);
    } catch (const std::exception& e) {
        return ers::make_error("Malformed JSON: {}", e.what());
    }

    template<class Node = TNode<default_traits>>
        requires json_detail::is_node_v<Node>
    [[nodiscard]]
    ers::Result<Node> try_from_file(const std::filesystem::path& path, std::size_t recursion_limit = json_detail::default_recursion_limit) try {
        return from_file<Node>(path, recursion_limit);
    } catch (const std::exception& e) {
        return ers::make_error("Cannot read '{}': {}", path.generic_string(), e.what());
    }


    extern template ERSLIB_EXPORT Json from_string<Json>(std::string_view, std::size_t);
    extern template ERSLIB_EXPORT Json from_file<Json>(const std::filesystem::path&, std::size_t);
    extern template ERSLIB_EXPORT ers::Result<Json> try_from_string<Json>(std::string_view, std::size_t);
    extern template ERSLIB_EXPORT ers::Result<Json> try_from_file<Json>(const std::filesystem::path&, std::size_t);
}

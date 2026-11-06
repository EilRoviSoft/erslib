#include "erslib/flux/migration/canonical.hpp"

// ers
#include <erslib/core/stdlib/ctype.hpp>
#include <erslib/core/util/string.hpp>


// Details ---------------------------------------------------------------------

namespace {
    constexpr auto npos = std::string_view::npos;


    constexpr bool is_high(char c) noexcept {
        return static_cast<unsigned char>(c) >= 0x80;
    }

    constexpr bool is_word_start(char c) noexcept {
        return ers::is_alpha(c) || c == '_' || is_high(c);
    }

    constexpr bool is_word(char c) noexcept {
        return ers::is_alnum(c) || c == '_' || is_high(c);
    }

    constexpr bool is_glue(char c) noexcept {
        return c == '(' || c == ')' || c == ',' || c == ';';
    }
}

namespace {
    size_t skip_quoted(std::string_view s, size_t i, char quote, bool backslash) {
        ++i;

        while (i < s.size()) {
            if (backslash && s[i] == '\\') {
                i += 2;
                continue;
            }

            if (s[i] == quote) {
                if (i + 1 < s.size() && s[i + 1] == quote) {
                    i += 2;
                    continue;
                }

                return i + 1;
            }

            ++i;
        }

        return s.size();
    }

    size_t skip_line_comment(std::string_view s, size_t i) {
        auto end = s.find('\n', i);
        return end == npos ? s.size() : end;
    }

    size_t skip_block_comment(std::string_view s, size_t i) {
        size_t depth = 0;

        while (i < s.size()) {
            auto rest = s.substr(i);

            if (rest.starts_with("/*")) {
                ++depth;
                i += 2;
            } else if (rest.starts_with("*/")) {
                i += 2;

                if (--depth == 0)
                    return i;
            } else {
                ++i;
            }
        }

        return s.size();
    }

    size_t dollar_tag_end(std::string_view s, size_t i) {
        size_t j = i + 1;

        if (j < s.size() && is_word_start(s[j])) {
            while (j < s.size() && is_word(s[j]))
                ++j;
        }

        return j < s.size() && s[j] == '$' ? j + 1 : npos;
    }

    size_t skip_dollar_quoted(std::string_view s, size_t i, size_t tag_end) {
        auto tag = s.substr(i, tag_end - i);
        auto close = s.find(tag, tag_end);

        return close == npos ? s.size() : close + tag.size();
    }
}


// Implementation --------------------------------------------------------------

namespace {
    class Canonizer {
    public:
        explicit Canonizer(std::string_view sql) : _sql(sql) {
            _out.reserve(sql.size());
        }

        std::string run() {
            while (_pos < _sql.size())
                _step();

            return std::move(_out);
        }


    private:
        std::string_view _sql;
        std::string _out;
        size_t _pos = 0;
        bool _space = false;

        char _peek(size_t offset = 0) const {
            return _pos + offset < _sql.size() ? _sql[_pos + offset] : '\0';
        }

        bool _at_line_start() const {
            return _pos == 0 || _sql[_pos - 1] == '\n';
        }

        void _put(std::string_view token) {
            bool need_space = _space
                && !_out.empty()
                && _out.back() != '\n'
                && !is_glue(_out.back())
                && !is_glue(token.front());

            if (need_space)
                _out += ' ';

            _space = false;
            _out += token;
        }

        void _take(size_t end) {
            _put(_sql.substr(_pos, end - _pos));
            _pos = end;
        }

        void _step() {
            char c = _peek();

            if (ers::is_space(c)) {
                _space = true;
                ++_pos;
            } else if (c == '-' && _peek(1) == '-') {
                _line_comment();
            } else if (c == '/' && _peek(1) == '*') {
                _pos = skip_block_comment(_sql, _pos);
                _space = true;
            } else if (c == '\'') {
                _take(skip_quoted(_sql, _pos, '\'', false));
            } else if (c == '"') {
                _take(skip_quoted(_sql, _pos, '"', false));
            } else if (c == '$') {
                _dollar();
            } else if (is_word_start(c)) {
                _word();
            } else {
                _take(_pos + 1);
            }
        }

        void _line_comment() {
            auto end = skip_line_comment(_sql, _pos);
            auto body = ers::util::trim(_sql.substr(_pos + 2, end - _pos - 2));

            if (_at_line_start() && body.starts_with("flux:")) {
                _out += '\n';
                _out += "--flux:";
                _out += ers::util::trim(body.substr(5));
                _out += '\n';
                _space = false;
            } else {
                _space = true;
            }

            _pos = end;
        }

        void _dollar() {
            auto tag_end = dollar_tag_end(_sql, _pos);

            if (tag_end == npos) {
                _take(_pos + 1);
                return;
            }

            _take(skip_dollar_quoted(_sql, _pos, tag_end));
        }

        void _word() {
            size_t end = _pos;

            while (end < _sql.size() && is_word(_sql[end]))
                ++end;

            auto word = ers::to_lower(_sql.substr(_pos, end - _pos));
            bool escape_string = word == "e" && end < _sql.size() && _sql[end] == '\'';

            _put(word);
            _pos = end;

            if (escape_string) {
                auto literal_end = skip_quoted(_sql, _pos, '\'', true);

                _out.append(_sql.substr(_pos, literal_end - _pos));
                _pos = literal_end;
            }
        }
    };
}


std::string flux::impl::canonical_sql(std::string_view sql) {
    return Canonizer(sql).run();
}

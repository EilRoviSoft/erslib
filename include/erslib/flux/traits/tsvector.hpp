#pragma once

// std
#include <string>
#include <string_view>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/flux/sql/value.hpp>


// Definition ------------------------------------------------------------------

namespace flux {
    class tsvector {
    public:
        tsvector() = default;

        static tsvector from_text(std::string text) {
            return tsvector(std::move(text));
        }

        std::string_view text() const noexcept {
            return _text;
        }

        bool empty() const noexcept {
            return _text.empty();
        }

        bool operator==(const tsvector&) const = default;


    private:
        explicit tsvector(std::string text) :
            _text(std::move(text)) {
        }

        std::string _text;
    };
}


// Traits ----------------------------------------------------------------------

template<>
struct flux::impl::sql_type<flux::tsvector> {
    static constexpr std::string_view name = "TSVECTOR";
    static constexpr bool nullable = false;
};

template<>
struct flux::impl::sql_value<flux::tsvector> {
    static std::string to_text(const flux::tsvector& what) {
        return std::string(what.text());
    }

    static void bind(pqxx::params& out, const flux::tsvector& what) {
        out.append(what.text());
    }

    static flux::tsvector read(const auto& field) {
        return flux::tsvector::from_text(field.template as<std::string>());
    }
};

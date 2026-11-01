#pragma once

// std
#include <exception>
#include <functional>
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::impl {
    template<typename T, auto Try, auto Maybe>
    struct parse_triple {
        static Result<T> try_parse(std::string_view text) {
            return std::invoke(Try, text);
        }

        static maybe<T> maybe_parse(std::string_view text) {
            return std::invoke(Maybe, text);
        }
    };

    template<typename T, auto Plain>
    struct parse_from_throwing {
        static Result<T> try_parse(std::string_view text) try {
            return std::invoke(Plain, text);
        } catch (const std::exception& e) {
            return make_error("{}", e.what());
        }

        static maybe<T> maybe_parse(std::string_view text) try {
            return std::invoke(Plain, text);
        } catch (...) {
            return none;
        }
    };
}


namespace ers {
    using impl::parse_triple;
    using impl::parse_from_throwing;
}

#pragma once

// ers
#include <erslib/core/exception.hpp>


namespace ers::impl {
    template<HasFromChars T, std::invocable<std::errc> OnError>
    std::invoke_result_t<OnError, std::errc> parse_number(std::string_view text, int base, OnError&& on_error) {
        T out;

        if (auto r = std::from_chars(text.begin(), text.end(), out, base); !r)
            return std::invoke(std::forward<OnError>(on_error), r.ec);

        return out;
    }

    template<typename T>
    std::string describe_number_error(std::string_view text, std::errc ec) {
        switch (ec) {
            case std::errc::result_out_of_range:
                return std::format("String '{}' when converted into '{}' is out of range.",
                    text, ers::meta::type_name_v<T>);

            default:
                return std::format("String '{}' can't be converted into '{}'.",
                    text, ers::meta::type_name_v<T>);
        }
    }
}


namespace ers::impl {
    template<HasFromChars T>
    T number_from_string(std::string_view text, int base) {
        return parse_number<T>(text, base, [text](std::errc ec) -> T {
            std::string message = describe_number_error<T>(text, ec);

            switch (ec) {
                case std::errc::result_out_of_range:
                    throw make_out_of_range_error_with_trace(message);

                default:
                    throw make_conversion_error(message);
            }
        });
    }

    template<HasFromChars T>
    Result<T> try_number_from_string(std::string_view text, int base) {
        return parse_number<T>(text, base, [text](std::errc ec) -> Result<T> {
            return make_error(describe_number_error<T>(text, ec));
        });
    }

    template<HasFromChars T>
    maybe<T> maybe_number_from_string(std::string_view text, int base) {
        return parse_number<T>(text, base, [](std::errc) -> maybe<T> {
            return none;
        });
    }
}

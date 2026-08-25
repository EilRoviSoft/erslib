#pragma once

// std
#include <charconv>
#include <concepts>

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::impl {
    template<typename T>
    concept HasFromChars = requires(const char* first, const char* last, T& v) {
        { std::from_chars(first, last, v) } -> std::same_as<std::from_chars_result>;
    };


    template<HasFromChars T>
    Result<T> number_from_chars(std::string_view source) {
        T out;
        
        if (auto r = std::from_chars(source.begin(), source.end(), out); !r) {
            switch (r.ec) {
                case std::errc::result_out_of_range:
                    return make_error("String '{}' when converted into '{}' is out of range.",
                        source, ers::meta::type_name_v<T>);

                default:
                    return make_error("String '{}' can't be converted into '{}'.",
                        source, ers::meta::type_name_v<T>);
            }
        }

        return out;
    }
}


namespace ers {
    using impl::number_from_chars;
}

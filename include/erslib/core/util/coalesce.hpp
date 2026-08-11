#pragma once

// std
#include <tuple>
#include <type_traits>

// boost
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>


namespace ers::util {
    template<typename T, typename... Ts>
    constexpr auto coalesce(T v, Ts... rest) {
        std::common_type_t<T, Ts...> result = v;
        ((result || ((result = rest))), ...);
        return result;
    }

    template<class T, class... Fs>
    constexpr auto coalesce_lazy(T v, Fs... fs) {
        std::common_type_t<T, std::invoke_result_t<Fs&>...> result = v;
        std::ignore = (static_cast<bool>(result) || ... || static_cast<bool>(result = fs()));
        return result;
    }
}

#define ERS_CO_WRAP(S, DATA, ELEM) [&]{ return (ELEM); }

#define ERS_COALESCE(FIRST, ...) ers::util::coalesce_lazy((FIRST) __VA_OPT__(, BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(ERS_CO_WRAP, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))))

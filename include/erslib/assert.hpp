#pragma once

// std
#include <format>
#include <string>

// ers
#include <erslib/util/string.hpp>


namespace ers::internal {
#ifdef ERS_ASSERT_INFO
    void print_message(std::string_view text);

    template<typename... Args>
    void custom_assert(bool condition, std::string_view fmt, Args&&... args) {
        if (condition) {
            std::string text = std::format(fmt, std::forward<Args>(args)...);
            print_message(text);
            std::abort();
        }
    }
#else
    template<typename... Args>
    void custom_assert(bool /*condition*/, std::string_view /*fmt*/, Args&&... /*args*/) {}
#endif
}


#define ERS_FMT_ASSERT(CONDITION, FMT, ...) \
	ers::internal::custom_assert(CONDITION, FMT, __VA_ARGS__)

#define ERS_MSR_ASSERT(CONDITION, MESSAGE) \
	ers::internal::custom_assert(CONDITION, ers::util::concat_literals<#CONDITION, " is violated. {}">().to_sv())

#define ERS_ASSERT(CONDITION) \
	ers::internal::custom_assert(CONDITION, ers::util::concat_literals<#CONDITION, " is violated.">().to_sv())

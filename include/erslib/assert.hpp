#pragma once

// std
#include <format>
#include <string>

// ers
#include <erslib/util/string.hpp>

namespace ers::internal {
#ifdef ERS_ASSERT_INFO
	void print_message(std::string_view text);
#else
	inline void print_message(std::string_view text) {}
#endif

	template<typename... Args>
	void assert(bool condition, std::string_view fmt, Args&&... args) {
		if (condition) {
			std::string text = std::format(fmt, std::forward<Args>(args)...);
			print_message(text);
			std::abort();
		}
	}
}

#define ERS_FMT_ASSERT(CONDITION, FMT, ...) \
	ers::internal::assert(CONDITION, FMT, __VA_ARGS__)

#define ERS_MSR_ASSERT(CONDITION, MESSAGE) \
	ers::internal::assert(CONDITION, ers::util::concat_literals(#CONDITION, " is violated. {}"))

#define ERS_ASSERT(CONDITION) \
	ers::internal::assert(CONDITION, ers::util::concat_literals(#CONDITION, " is violated."))

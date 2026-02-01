#pragma once

// std
#include <string>

namespace ers::util {
	template<typename T>
	constexpr std::string_view to_sv(const T& what) = delete("default specialization");

	template<typename T>
	std::string to_string(const T& what) = delete("default specialization");
}

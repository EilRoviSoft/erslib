#pragma once

// std
#include <string_view>

// classes

namespace ers::hashing {
	template<typename T>
	struct Std {
		constexpr size_t operator()(const T& what) const noexcept {
			return std::hash<T> {}(what);
		}
	};
}

namespace ers {
	template<typename T>
	constexpr hashing::Std<T> hash;
}

// functions

namespace ers::hashing {
	template<typename... TArgs>
	constexpr size_t combine(const TArgs&... args) noexcept {
		size_t r = 0;

		auto combine_step = [&r]<typename T>(const T& arg) {
			r ^= hash<T>(arg) + 0x9e3779b97f4a7c15 + (r << 6) + (r >> 2);
		};

		(combine_step(args), ...);

		return r;
	}
}

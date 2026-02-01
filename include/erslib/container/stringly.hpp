#pragma once

// std
#include <string>

// boost
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

// ers
#include <erslib/hashing/std.hpp>

namespace ers::container {
	struct stringly_hash {
		using is_transparent = void;

		size_t operator()(const std::string_view sv) const {
			return ers::hash<std::string_view>(sv);
		}
		size_t operator()(const std::string& str) const {
			return ers::hash<std::string>(str);
		}
		size_t operator()(const char* cstr) const {
			return ers::hash<std::string_view>(cstr);
		}
	};

	struct stringly_equal {
		using is_transparent = void;

		bool operator()(const std::string& a, const std::string& b) const noexcept {
			return a == b;
		}

		bool operator()(const std::string_view a, const std::string_view b) const noexcept {
			return a == b;
		}
		bool operator()(const std::string& a, const std::string_view b) const noexcept {
			return a == b;
		}
		bool operator()(const std::string_view a, const std::string& b) const noexcept {
			return a == b;
		}

		bool operator()(const std::string& a, const char* b) const noexcept {
			return a == b;
		}
		bool operator()(const char* a, const std::string& b) const noexcept {
			return a == b;
		}
	};
}

namespace ers {
	template<typename TAllocator = std::allocator<std::string>>
	using stringly_set = boost::unordered_set<
		std::string,
		container::stringly_hash,
		container::stringly_equal,
		TAllocator
	>;

	template<typename TVal>
	using stringly_allocator = std::allocator<std::pair<const std::string, TVal>>;

	template<typename TVal, typename TAllocator = stringly_allocator<TVal>>
	using stringly_map = boost::unordered_map<
		std::string,
		TVal,
		container::stringly_hash,
		container::stringly_equal,
		TAllocator
	>;
}

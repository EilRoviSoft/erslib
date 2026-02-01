#pragma once

// std
#include <string>
#include <string_view>

namespace ers {
	template<typename T>
	concept StringlyConvertible = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>;
}

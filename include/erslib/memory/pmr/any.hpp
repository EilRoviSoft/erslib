#pragma once

// ers
#include <erslib/memory/pmr/any-impl.hpp>

constexpr size_t Size = 16;
constexpr size_t Align = alignof(std::max_align_t);

namespace ers::pmr {
	class TAny : public TAnyImpl<Size, Align> {
	public:
		// Constructors

		TAny() noexcept :
			TAnyImpl() {
		}
	};
}

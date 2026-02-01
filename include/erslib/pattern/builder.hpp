#pragma once

namespace ers::pattern {
	template<typename T>
	class IBuilder {
	public:
		virtual ~IBuilder() = default;

		[[nodiscard]] virtual bool check() const noexcept {
			return true;
		}

		virtual T build() && = 0;
	};
}

#include "erslib/assert.hpp"

// std
#include <concepts>
#include <fstream>
#include <iostream>
#include <vector>

// In case we have information about assertions

#ifdef ERS_ASSERT_INFO
namespace ers::internal {
	class AssertTool {
		AssertTool() = default;

	public:
		bool is_initialized = false;
		std::vector<std::ostream*> enabled;

		static AssertTool& instance() {
			static AssertTool instance;
			return instance;
		}

		template<typename... Args>
			requires (std::derived_from<Args, std::ostream>, ...) && (!std::is_const_v<Args>, ...)
		void init(Args&&... args) {
			enabled.reserve(sizeof...(Args));

			auto iteration = [&]<typename T>(T&& what) {
				if constexpr (std::is_reference_v<T>) {
					enabled.emplace_back(&what);
				} else {
					auto ptr = std::make_unique<std::ostream>(std::forward<T>(what));
					auto ref = m_allocated.emplace_back(std::move(ptr));
					enabled.emplace_back(&ref);
				}
			};

			(iteration(args), ...);

			is_initialized = true;
		}

	protected:
		std::vector<std::unique_ptr<std::ostream>> m_allocated;
	};

	void print_message(std::string_view text) {
		auto& tool = AssertTool::instance();
		
		if (!tool.is_initialized)
			tool.init(std::cerr, std::fstream("log.txt"));

		for (const auto& it : tool.enabled)
			*it << text << '\n' << std::flush;
	}
}
#endif

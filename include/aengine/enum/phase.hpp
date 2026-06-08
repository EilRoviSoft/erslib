#pragma once

// ers
#include <erslib/convert/string.hpp>
#include <erslib/util/string.hpp>


namespace aengine {
    enum class Phase {
        Unknown = 0,
        Settings,
        Data,
        Scripts,
        Max
    };
}


template<>
struct ers::convert::from_string_backend<aengine::Phase> {
    using enum aengine::Phase;

    static constexpr auto conversion_table = std::to_array<std::pair<std::string_view, aengine::Phase>>({
        { "settings", Settings },
        { "data", Data },
        { "scripts", Scripts }
    });

    constexpr aengine::Phase constexpr_value(std::string_view source) const noexcept {
        for (const auto& [k, v] : conversion_table) {
            if (util::starts_with_seq(source, k, '-'))
                return v;
        }

        return Unknown;
    }
};

#pragma once

#ifdef ERSLIB_HAS_REFLECTION


// std
#include <concepts>
#include <format>
#include <meta>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/meta/impl/funcsig.hpp>


namespace ers::impl {
    struct AutoFormat {};
}

template<std::derived_from<ers::impl::AutoFormat> T>
struct std::formatter<T> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
            throw ers::format_error("Invalid struct format spec");
        return it;
    }

    auto format(const T& value, std::format_context& ctx) const {
        auto out = std::format_to(ctx.out(), "{} = [", ers::impl::funcsig<T>());
        bool first = true;

        template for (constexpr auto m : std::define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
            if (!first)
                out = std::format_to(out, ", ");
            out = std::format_to(out, "{} = {}", std::meta::identifier_of(m), value.[:m:]);
            first = false;
        }

        *out++ = ']';
        return out;
    }
};


// Exports

namespace ers {
    using impl::AutoFormat;
}


#endif

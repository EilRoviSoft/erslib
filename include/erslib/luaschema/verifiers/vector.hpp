#pragma once

// std
#include <vector>

// ers
#include <erslib/luaschema/impl/verifier.hpp>


template<typename T>
struct ers::luaschema::verifier_t<std::vector<T>> {
    Status exec(impl::verify_context& ctx, sol::object obj) const {
        if (!obj.is<sol::table>())
            return make_error("Expected array table");

        auto arr = obj.as<sol::table>();

        verifier_t<T> elem;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (auto s = elem.exec(ctx, arr[i + 1]); !s)
                return s;
        }

        return ok;
    }
};

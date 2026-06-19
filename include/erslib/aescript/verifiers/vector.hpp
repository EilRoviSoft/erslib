#pragma once

// std
#include <vector>

// aescript
#include <erslib/aescript/impl/verifier.hpp>


template<typename T>
struct aescript::verifier_t<std::vector<T>> {
    ers::Status exec(verify_context& ctx, sol::object obj) const {
        if (!obj.is<sol::table>())
            return ers::make_error(ers::Severity::Error, "Expected array table");

        auto arr = obj.as<sol::table>();

        verifier_t<T> elem;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (auto s = elem.exec(ctx, arr[i + 1]); !s)
                return s;
        }
        
        return ers::ok;
    }
};

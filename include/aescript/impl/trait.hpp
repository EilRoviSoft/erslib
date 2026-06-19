#pragma once

// std
#include <concepts>
#include <string>


namespace aescript::internal {
    template<typename T>
    concept SolInnateType =
        std::same_as<T, std::string>
        || std::same_as<T, bool>
        || std::integral<T>
        || std::floating_point<T>;


    template<SolInnateType T>
    consteval sol::type sol_type_for() {
        if constexpr (std::same_as<T, bool>)
            return sol::type::boolean;
        else if constexpr (std::same_as<T, std::string>)
            return sol::type::string;
        else if constexpr (std::floating_point<T> || std::integral<T>)
            return sol::type::number;
        else
            return sol::type::nil;
    }


    template<SolInnateType T>
    [[nodiscard]]
    ers::Status check_type(sol::object obj) {
        static constexpr sol::type expected_type = internal::sol_type_for<T>();

        if (!obj.valid()) {
            return ers::make_error(
                ers::Severity::Error,
                "Object is not valid or nil"
            );
        }

        if (obj.get_type() != expected_type) {
            return ers::make_error(
                ers::Severity::Error,
                "Object has unexpected type"
            );
        }

        return ers::ok;
    }
}


namespace aescript::internal {
    template<typename T>
    concept HasLayoutDescriptor = requires() {
        { T::get_layout() } -> std::same_as<const Layout&>;
    };

    template<typename T>
    concept HasVerifier = requires(verify_context ctx, sol::object obj) {
        { verifier_t<T> {}.exec(ctx, obj) } -> std::convertible_to<ers::Status>;
    };

    template<typename T>
    concept HasParser = requires(verify_context ctx, sol::object obj, void* dst) {
        { parser_t<T> {}.exec(ctx, obj, dst) } -> std::convertible_to<ers::Status>;
    };
}

#pragma once

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/type/result.hpp>

// aescript
#include <aescript/impl/property_context.hpp>


namespace aescript {
    // TODO: add 'make_property' instead of 'make_unique<DerivedFromProperty>'
    using FieldPropertyPtr = std::unique_ptr<class IProperty>;

    class IProperty {
    public:
        // Member functions

        explicit IProperty(size_t precedence) :
            _precedence(precedence) {
        }

        virtual ~IProperty() = default;


        // Checkers

        [[nodiscard]]
        virtual ers::Status verify([[maybe_unused]] property_context& ctx, sol::table table, std::string_view field) const = 0;


        // Accessors

        [[nodiscard]]
        size_t precedence() const { return _precedence; }


        // Misc

        [[nodiscard]]
        virtual FieldPropertyPtr clone() const = 0;


    private:
        size_t _precedence;
    };
}

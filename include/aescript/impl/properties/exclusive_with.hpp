#pragma once

// std
#include <initializer_list>
#include <string>
#include <vector>

// aescript
#include <aescript/impl/property.hpp>


namespace aescript {
    class ExclusiveWithProperty : public IProperty {
    public:
        // Member functions

        ExclusiveWithProperty(std::initializer_list<std::string_view> il);


        // Checkers

        [[nodiscard]]
        ers::Status verify([[maybe_unused]] property_context& ctx, sol::table table, std::string_view field) const override;


        // Misc

        FieldPropertyPtr clone() const override;


    private:
        std::vector<std::string> _incompatible_fields;


        ExclusiveWithProperty();
    };


    namespace properties {
        FieldPropertyPtr exclusive_with(std::initializer_list<std::string_view> il);
    }
}

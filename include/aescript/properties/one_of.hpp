#pragma once

// std
#include <string>
#include <vector>

// aescript
#include <aescript/impl/descriptor.hpp>


namespace aescript {
    class OneOfProperty : public IDescriptor {
    public:
        // Member functions

        explicit OneOfProperty(std::vector<std::string> names);


        // Executors

        [[nodiscard]]
        ers::Status verify(sol::table table) const override;


        // Misc

        [[nodiscard]]
        DescriptorPtr clone() const override;


    private:
        std::vector<std::string> _names;
    };


    namespace properties {
        DescriptorPtr one_of(std::initializer_list<std::string_view> il);
    }
}

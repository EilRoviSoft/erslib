#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/aescript/impl/descriptor.hpp>


namespace aescript::impl {
    class OneOfProperty : public IDescriptor {
    public:
        // Member functions

        explicit OneOfProperty(std::vector<std::string> names);


        // Executors

        ers::Status verify(sol::table table) const override;


    private:
        std::vector<std::string> _names;
    };


    namespace properties {
        Descriptor one_of(std::initializer_list<std::string_view> il);
    }
}

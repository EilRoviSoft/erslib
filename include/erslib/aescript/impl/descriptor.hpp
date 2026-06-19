#pragma once

// std
#include <memory>

// sol
#include "sol/table.hpp"

// ers
#include "erslib/core/type/result.hpp"


namespace aescript {
    using DescriptorPtr = std::unique_ptr<class IDescriptor>;

    class IDescriptor {
    public:
        // Member functions

        IDescriptor() = default;

        virtual ~IDescriptor() = default;


        // Executors

        [[nodiscard]]
        virtual ers::Status verify(sol::table table) const = 0;


        // Misc

        [[nodiscard]]
        virtual DescriptorPtr clone() const = 0;
    };
}

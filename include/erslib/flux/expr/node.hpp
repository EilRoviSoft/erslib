#pragma once

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/sql/context.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    enum class EPrecedence : u8 {
        Or, And, Not, Is, Comparison, Like, Other, AddSub, MulDiv, Unary, Cast, Primary,
    };


    class ERSLIB_EXPORT IExprNode {
    public:
        // Member functions

        virtual ~IExprNode() = default;


        // Observers

        virtual EPrecedence precedence() const noexcept { return EPrecedence::Primary; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;
    };
    using ExprNodePtr = std::shared_ptr<const IExprNode>;
}

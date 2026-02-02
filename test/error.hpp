#pragma once

// std
#include <limits>

// ers
#include <erslib/error/include.hpp>

class ErrorModule : public testing::Test {
public:
    static ers::Result<double> safe_divide(double l, double r) {
        if (r < std::numeric_limits<double>::epsilon())
            return ers::Unexpected<ers::Error>(ers::Severity::Crit, "math_error", "Divided by 0");
        return l / r;
    }
};

TEST_F(ErrorModule, SuccessfulDivision) {
    EXPECT_TRUE(safe_divide(1.0, 2.0));
}

TEST_F(ErrorModule, DivisionByZero) {
    EXPECT_FALSE(safe_divide(1.0, 0.0));
}

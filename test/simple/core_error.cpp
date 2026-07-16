// doctest
#include <doctest/doctest.h>

// std
#include <cmath>
#include <limits>

// ers
#include <erslib/core/type/result.hpp>


namespace {
    bool equals(double l, double r) {
        return std::fabs(l - r) < std::numeric_limits<double>::epsilon();
    }

    ers::Result<double> safe_divide(double l, double r) {
        if (equals(r, 0.0))
            return ers::make_error(ers::Severity::Error, "Division by zero: {} / {}", l, r);
        return l / r;
    }
}


TEST_CASE("division") {
    SUBCASE("successful") {
        auto check = [](double n, double d, double e) {
            auto r = safe_divide(n, d);
            CHECK(r.has_value());
            REQUIRE(*r == doctest::Approx(e));
        };

        check(1.0, 2.0, 0.5);
        check(3.0, 1.0, 3.0);
    }

    SUBCASE("by zero") {
        auto check = [](double n, double d) {
            auto r = safe_divide(n, d);
            CHECK(r.has_error());
            //MESSAGE(r.error().to_string());
        };

        check(1.0, 0.0);
        check(3.0, 0.0);
    }
}

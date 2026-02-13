// std
#include <limits>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// ers
#include <erslib/error/include.hpp>


bool equals(double l, double r) {
    return std::fabs(l - r) < std::numeric_limits<double>::epsilon();
}

ers::Result<double> safe_divide(double l, double r) {
    if (equals(r, 0.0)) {
        return ers::Unexpected<ers::Error>(
            ers::Severity::Crit,
            "math_error",
            std::format("Division by zero: {} / {}", l, r)
        );
    }

    return l / r;
}


TEST_CASE("division", "[error]") {
    SECTION("successful") {
        auto check = [](double n, double d, double e) {
            auto r = safe_divide(n, d);
            CHECK(r.has_value());
            REQUIRE_THAT(*r, Catch::Matchers::WithinRel(e));
        };

        check(1.0, 2.0, 0.5);
        check(3.0, 1.0, 3.0);
    }

    SECTION("by zero") {
        auto check = [](double n, double d) {
            auto r = safe_divide(n, d);
            CHECK(r.has_error());
            SUCCEED(r.error().to_string());
        };

        check(1.0, 0.0);
        check(3.0, 0.0);
    }
}

// catch2
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

// std
#include <cmath>
#include <limits>
#include <string>

// ers
#include <erslib/core/type/result.hpp>

// test
#include "support/tags.hpp"


namespace {
    bool equals(double l, double r) {
        return std::fabs(l - r) < std::numeric_limits<double>::epsilon();
    }

    ers::Result<double> safe_divide(double l, double r) {
        if (equals(r, 0.0))
            return ers::make_error("Division by zero: {} / {}", l, r);
        return l / r;
    }

    ers::Result<double> read_ratio(double l, double r) {
        auto ratio = ERS_TRY(safe_divide(l, r).context("Ratio {} / {}", l, r));
        return ratio * 100;
    }

    ers::Status read_config() {
        ERS_TRY(read_ratio(1.0, 0.0).with_context([] { return std::string("File 'config.json'"); }));
        return ers::ok;
    }
}


TEST_CASE("division", TEST_TAGS) {
    SECTION("ok") {
        auto check = [](double n, double d, double e) {
            auto r = safe_divide(n, d);
            CHECK(r.has_value());
            REQUIRE(*r == Catch::Approx(e));
        };

        check(1.0, 2.0, 0.5);
        check(3.0, 1.0, 3.0);
    }

    SECTION("zero") {
        auto check = [](double n, double d) {
            auto r = safe_divide(n, d);
            CHECK(r.has_error());
        };

        check(1.0, 0.0);
        check(3.0, 0.0);
    }
}

TEST_CASE("context", TEST_TAGS) {
    SECTION("value") {
        auto r = read_ratio(1.0, 4.0);
        REQUIRE(r.has_value());
        CHECK(*r == Catch::Approx(25.0));
    }

    SECTION("outermost first") {
        auto s = read_config();
        REQUIRE(s.has_error());

        CHECK(s.error().message() == "Division by zero: 1 / 0");
    }

    SECTION("no prefix") {
        auto r = safe_divide(1.0, 0.0);
        REQUIRE(r.has_error());
    }
}

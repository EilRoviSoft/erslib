// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string_view>

// ers
#include <erslib/core/type/version.hpp>

// test
#include "support/test_case.hpp"


ERS_TEST_CASE("parse", "[core]") {
    std::string_view s;
    ers::version_t v;


    SECTION("1.0.1") {
        s = "1.0.1";
        v = { .major = 1, .minor = 0, .patch = 1 };
    }

    SECTION("26.3.19") {
        s = "26.3.19";
        v = { .major = 26, .minor = 3, .patch = 19 };
    }


    auto r = ers::version_t::parse(s);
    REQUIRE(r);
    CHECK(*r == v);
}

ERS_TEST_CASE("malformed", "[core]") {
    CHECK_FALSE(ers::version_t::parse("1.0"));
    CHECK_FALSE(ers::version_t::parse("1"));
    CHECK_FALSE(ers::version_t::parse(""));
    CHECK_FALSE(ers::version_t::parse("a.b.c"));
}

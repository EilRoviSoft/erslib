// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/core/type/version.hpp>


TEST_CASE("version: parse") {
    std::string_view s;
    ers::version_t v;


    SUBCASE("1.0.1") {
        s = "1.0.1";
        v = { .major = 1, .minor = 0, .patch = 1 };
    }

    SUBCASE("26.3.19") {
        s = "26.3.19";
        v = { .major = 26, .minor = 3, .patch = 19 };
    }


    auto r = ers::version_t::parse(s);
    REQUIRE(r);
    CHECK(*r == v);
}

TEST_CASE("version: malformed input is an error") {
    CHECK_FALSE(ers::version_t::parse("1.0"));
    CHECK_FALSE(ers::version_t::parse("1"));
    CHECK_FALSE(ers::version_t::parse(""));
    CHECK_FALSE(ers::version_t::parse("a.b.c"));
}

// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/type/version.hpp>


TEST_CASE("version", "[convert]") {
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


    REQUIRE(*ers::convert::from_str<ers::version_t>(s) == v);
    REQUIRE(ers::convert::to_str(v) == s);
}

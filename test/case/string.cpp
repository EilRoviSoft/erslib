// std
#include <string>

// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/util/string.hpp>


TEST_CASE("path", "[string]") {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

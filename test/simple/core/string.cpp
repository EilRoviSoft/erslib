// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>

// ers
#include <erslib/core/util/string.hpp>

// test
#include "support/test_case.hpp"


ERS_TEST_CASE("path", "[core]") {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

ERS_TEST_CASE("trim", "[core]") {
    REQUIRE(ers::util::trim("  a b \t\n") == "a b");
    REQUIRE(ers::util::trim(" \t ").empty());
    REQUIRE(ers::util::trim_left("  x ") == "x ");
    REQUIRE(ers::util::trim_right("  x ") == "  x");
}

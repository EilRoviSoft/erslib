// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>

// ers
#include <erslib/core/util/string.hpp>

// test
#include "support/tags.hpp"


TEST_CASE("path", TEST_TAGS) {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

TEST_CASE("trim", TEST_TAGS) {
    REQUIRE(ers::util::trim("  a b \t\n") == "a b");
    REQUIRE(ers::util::trim(" \t ").empty());
    REQUIRE(ers::util::trim_left("  x ") == "x ");
    REQUIRE(ers::util::trim_right("  x ") == "  x");
}

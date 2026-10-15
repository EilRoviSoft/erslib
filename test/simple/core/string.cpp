// doctest
#include <doctest/doctest.h>

// std
#include <string>

// ers
#include <erslib/core/util/string.hpp>


TEST_CASE("path") {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

TEST_CASE("trim") {
    REQUIRE(ers::util::trim("  a b \t\n") == "a b");
    REQUIRE(ers::util::trim(" \t ").empty());
    REQUIRE(ers::util::trim_left("  x ") == "x ");
    REQUIRE(ers::util::trim_right("  x ") == "  x");
}

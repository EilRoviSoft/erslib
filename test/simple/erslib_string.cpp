// std
#include <string>

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/util/string.hpp>


TEST_CASE("path") {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

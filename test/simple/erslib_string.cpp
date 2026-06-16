// doctest
#include <doctest/doctest.h>

// std
#include <string>

// ers
#include <erslib/util/string.hpp>


TEST_CASE("path") {
    REQUIRE(ers::util::replace("/home/user", "/", ".") == ".home.user");
}

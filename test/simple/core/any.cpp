// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/memory.hpp>

// test
#include "support/tags.hpp"


TEST_CASE("any", TEST_TAGS) {
    ers::Any example(std::in_place_type<std::string>, "hello world!");
}

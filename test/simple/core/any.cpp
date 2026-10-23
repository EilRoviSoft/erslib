// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/memory.hpp>

// test
#include "support/test_case.hpp"


ERS_TEST_CASE("any", "[core]") {
    ers::Any example(std::in_place_type<std::string>, "hello world!");
}

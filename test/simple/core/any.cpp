// doctest
#include <doctest/doctest.h>

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/memory.hpp>


TEST_CASE("testing TAny") {
    ers::Any example(std::in_place_type<std::string>, "hello world!");
}

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/core/memory/any.hpp>


TEST_CASE("testing TAny") {
    ers::Any example(std::in_place_type<std::string>, "hello world!");
}

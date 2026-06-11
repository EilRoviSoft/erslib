#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/memory/any.hpp>


TEST_CASE("testing TAny") {
    ers::Any example(std::in_place_type<std::string>, "hello world!");
}

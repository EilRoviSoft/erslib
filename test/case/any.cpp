// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/memory/pmr/any.hpp>


TEST_CASE("testing TAny", "[any]") {
    ers::pmr::Any example(std::in_place_type<std::string>, "hello world!");
}

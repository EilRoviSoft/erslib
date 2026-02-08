// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/meta/type_name.hpp>

template<typename T>
struct sample_t {};

TEST_CASE("testing meta", "[meta]") {
    REQUIRE(ers::meta::type_name<int>() == "int");
    REQUIRE(ers::meta::type_name<float>() == "float");
    REQUIRE(ers::meta::type_name<long double>() == "long double");
    REQUIRE(ers::meta::type_name<sample_t<int>>() == "sample_t<int>");
}

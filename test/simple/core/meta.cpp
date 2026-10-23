// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/core/meta.hpp>

// test
#include "support/test_case.hpp"


template<typename T>
struct sample_t {};


ERS_TEST_CASE("meta", "[core]") {
    REQUIRE(ers::meta::type_name_v<int> == "int");
    REQUIRE(ers::meta::type_name_v<float> == "float");
    REQUIRE(ers::meta::type_name_v<long double> == "long double");
    REQUIRE(ers::meta::type_name_v<sample_t<int>> == "sample_t<int>");
}

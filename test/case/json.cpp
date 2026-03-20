// catch2
#include <catch2/catch_test_macros.hpp>

// contrib
#include <contrib/json.hpp>


TEST_CASE("assignment", "[json]") {
    utl::Json obj;

    obj["hello"] = "world";
}

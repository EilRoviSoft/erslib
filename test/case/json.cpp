// catch2
#include <catch2/catch_test_macros.hpp>

// contrib
#include <contrib/json.hpp>
#include <contrib/json_util.hpp>


TEST_CASE("assignment", "[json]") {
    utl::Json obj;

    obj["hello"] = "world";
}


TEST_CASE("require_field", "[json]") {
    utl::Json obj;

    obj["int"] = 42;
    obj["float"] = 3.14;
    obj["str"] = "hello world";

    SECTION("Require") {
        ers::util::Schema schema(obj);

        schema.require<utl::Json::integral_type>("int");
        schema.require<utl::Json::floating_type>("float");
        schema.require<utl::Json::string_type>("str");

        REQUIRE(schema.finalize());
    }
}

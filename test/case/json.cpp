// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <contrib/json.hpp>
#include <erslib/io/json_schema.hpp>


using integral = utl::Json::integral_type;
using floating = utl::Json::floating_type;
using string = utl::Json::string_type;


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
        ers::JsonSchema schema(obj);

        schema.require<integral>("int");
        schema.require<floating>("float");
        schema.require<string>("str");

        REQUIRE(schema.finalize());
    }

    SECTION("Require and Write") {
        ers::JsonSchema schema(obj);

        integral i;
        floating f;
        string s;

        schema.require_and_write("int", i);
        schema.require_and_write("float", f);
        schema.require_and_write("str", s);

        REQUIRE(schema.finalize());
        REQUIRE(i == obj["int"].as<integral>());
        REQUIRE(f - obj["float"].as<floating>() < std::numeric_limits<float>::epsilon());
        REQUIRE(s == obj["str"].as<string>());
    }
}

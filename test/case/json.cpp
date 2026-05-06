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


    std::string text;


    auto s = ers::util::Schema(obj)
        .require<int64_t>("int")
        .require<double_t>("float")
        .require_and_write("str", text)
        .finalize();
}

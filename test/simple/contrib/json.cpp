// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/contrib/json.hpp>


TEST_CASE("construction picks the right type") {
    CHECK(utl::Json().is_null());
    CHECK(utl::Json(nullptr).is_null());
    CHECK(utl::Json(true).is_bool());
    CHECK(utl::Json(5).is_integral());
    CHECK(utl::Json(5u).is_integral());
    CHECK(utl::Json(1.5).is_floating());
    CHECK(utl::Json(1.5f).is_floating());
    CHECK(utl::Json("x").is_string());
    CHECK(utl::Json(std::string_view("x")).is_string());
    CHECK(utl::Json(std::string("x")).is_string());
    CHECK(utl::Json(utl::Object{}).is_object());
    CHECK(utl::Json(utl::Array{}).is_array());
}

TEST_CASE("type matches the variant index") {
    CHECK(utl::Json().type() == utl::ENodeType::None);
    CHECK(utl::Json(utl::Object{}).type() == utl::ENodeType::Object);
    CHECK(utl::Json(true).type() == utl::ENodeType::Bool);
}

TEST_CASE("integral range is checked") {
    CHECK_THROWS(utl::Json(std::numeric_limits<u64>::max()));
}

TEST_CASE("assignment from a value") {
    utl::Json json;

    json = 5;
    CHECK(json.as_integral() == 5);

    json = "x";
    CHECK(json.as_string() == "x");
}

TEST_CASE("assignment from a child") {
    utl::Array array;
    array.emplace_back(utl::Array { utl::Json(1), utl::Json(2) });

    utl::Json json = std::move(array);

    json = std::move(json.as_array()[0]);
    REQUIRE(json.is_array());
    CHECK(json.as_array().size() == 2);

    json = json.as_array()[1];
    CHECK(json.as_integral() == 2);
}

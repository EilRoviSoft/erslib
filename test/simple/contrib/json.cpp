// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/contrib/json.hpp>


// Basic tests

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


// Advanced tests with custom traits

#include <map>


struct test_key {
    std::string value;

    auto operator<=>(const test_key&) const = default;
};

template<>
struct utl::string_adaptor<test_key> {
    [[nodiscard]]
    static test_key from_utf8(std::string&& value) {
        return { std::move(value) };
    }

    [[nodiscard]]
    static std::string_view to_utf8(const test_key& value) noexcept {
        return value.value;
    }
};


struct plain_map_traits : utl::default_traits {
    template<class Node>
    using object_type = std::map<std::string, Node>;
};

struct custom_key_traits : utl::default_traits {
    template<class Node>
    using object_type = std::map<test_key, Node>;
};


TEST_CASE("operator[] creates an object from null and inserts keys") {
    utl::Json json;

    json["a"] = 1;
    json["b"]["c"] = "x";

    REQUIRE(json.is_object());
    CHECK(json.as_object().size() == 2);
    CHECK(json["a"].as_integral() == 1);
    CHECK(json["b"]["c"].as_string() == "x");
}

TEST_CASE("operator[] on a non-object throws") {
    utl::Json json = 5;
    CHECK_THROWS_AS(json["a"], ers::conversion_error);
}

TEST_CASE("const access throws on a missing key or a non-object") {
    utl::Json json;
    json["a"] = 1;

    const auto& view = json;
    CHECK(view["a"].as_integral() == 1);
    CHECK(view.at("a").as_integral() == 1);
    CHECK_THROWS_AS(view.at("b"), ers::out_of_range_error);
    CHECK_THROWS_AS(view["b"], ers::out_of_range_error);

    const utl::Json number = 5;
    CHECK_THROWS_AS(number.at("a"), ers::conversion_error);
}

TEST_CASE("try_at and contains don't throw") {
    utl::Json json;
    json["a"] = 1;

    CHECK(json.contains("a"));
    CHECK_FALSE(json.contains("b"));
    CHECK(json.try_at("a"));
    CHECK_FALSE(json.try_at("b"));

    if (auto child = json.try_at("a"))
        *child = 2;
    CHECK(json["a"].as_integral() == 2);

    const utl::Json number = 5;
    CHECK_FALSE(number.contains("a"));
    CHECK_FALSE(number.try_at("a"));
}

TEST_CASE("assignment from a child through operator[]") {
    utl::Json json;
    json["value"]["inner"] = 1;
    json["other"] = 2;

    json = std::move(json["value"]);
    REQUIRE(json.is_object());
    CHECK(json["inner"].as_integral() == 1);
    CHECK_FALSE(json.contains("other"));

    json = json["inner"];
    CHECK(json.as_integral() == 1);
}

TEST_CASE("lookup without heterogeneous find") {
    utl::TNode<plain_map_traits> json;
    json["a"] = 1;

    CHECK(json.contains("a"));
    CHECK(json.at("a").as_integral() == 1);
}

TEST_CASE("custom key type goes through string_adaptor") {
    utl::TNode<custom_key_traits> json;
    json["a"] = 1;

    CHECK(json.as_object().begin()->first.value == "a");
    CHECK(json.contains("a"));
    CHECK(json.at("a").as_integral() == 1);
}

#ifdef _HAS_VALIJSON

// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <ranges>
#include <string>

// ers
#include <erslib/contrib/valijson.hpp>

// test
#include "support/tags.hpp"


using utl::operator""_json;


namespace {
    bool contains(std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) != std::string_view::npos;
    }

    utl::JsonValidator compile(const utl::Json& schema) {
        auto out = utl::JsonValidator::compile(schema);
        REQUIRE(out);
        return std::move(*out);
    }


    const auto user_schema = R"({
        "type": "object",
        "required": ["name", "age"],
        "properties": {
            "name": { "type": "string", "minLength": 1 },
            "age": { "type": "integer", "minimum": 0 },
            "role": { "enum": ["admin", "staff", "user"] },
            "tags": { "type": "array", "items": { "type": "string" }, "maxItems": 3 }
        }
    })"_json;
}


TEST_CASE("valid", TEST_TAGS) {
    const auto validator = compile(user_schema);

    CHECK(validator.validate(R"({"name": "Bob", "age": 30})"_json));
    CHECK(validator.validate(R"({"name": "Bob", "age": 30, "role": "staff", "tags": ["a", "b"]})"_json));
}

TEST_CASE("violations", TEST_TAGS) {
    const auto validator = compile(user_schema);
    const auto status = validator.validate(R"({"age": -1, "role": "root", "tags": ["a", 2]})"_json);

    REQUIRE_FALSE(status);

    const auto message = std::string(status.error().message());

    CHECK(contains(message, "name"));
    CHECK(contains(message, "/age"));
    CHECK(contains(message, "/role"));
    CHECK(contains(message, "/tags/1"));
}

TEST_CASE("integers", TEST_TAGS) {
    const auto integer = compile(R"({"type": "integer"})"_json);
    const auto number = compile(R"({"type": "number"})"_json);

    CHECK(integer.validate(utl::Json(5)));
    CHECK_FALSE(integer.validate(utl::Json(1.5)));
    CHECK(number.validate(utl::Json(5)));
    CHECK(number.validate(utl::Json(1.5)));
    CHECK_FALSE(number.validate(R"("5")"_json));
}

TEST_CASE("references", TEST_TAGS) {
    const auto validator = compile(R"({
        "definitions": { "id": { "type": "integer", "minimum": 1 } },
        "type": "array",
        "items": { "$ref": "#/definitions/id" }
    })"_json);

    CHECK(validator.validate("[1, 2, 3]"_json));
    CHECK_FALSE(validator.validate("[1, 0]"_json));
}


TEST_CASE("broken_schema", TEST_TAGS) {
    CHECK_FALSE(utl::JsonValidator::compile(R"({"allOf": 5})"_json));
    CHECK_FALSE(utl::JsonValidator::compile(R"({"maxLength": -1})"_json));
    CHECK_FALSE(utl::JsonValidator::compile(R"({"$ref": "#/definitions/missing"})"_json));
}

TEST_CASE("copy", TEST_TAGS) {
    const auto original = compile(user_schema);
    const auto copy = original;

    CHECK(copy.validate(R"({"name": "Bob", "age": 1})"_json));
    CHECK_FALSE(copy.validate(R"({"name": ""})"_json));
}


TEST_CASE("ordered_nodes", TEST_TAGS) {
    const auto schema = utl::from_string<utl::OrderedJson>(R"({"type": "object", "required": ["b", "a"]})");
    const auto validator = utl::JsonValidator::compile(schema);

    REQUIRE(validator);
    CHECK(validator->validate(utl::from_string<utl::OrderedJson>(R"({"b": 1, "a": 2})")));
    CHECK_FALSE(validator->validate(utl::from_string<utl::OrderedJson>(R"({"b": 1})")));
}

#endif

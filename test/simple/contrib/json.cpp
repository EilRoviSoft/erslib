// doctest
#include <doctest/doctest.h>

// std
#include <array>
#include <deque>
#include <expected>
#include <filesystem>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

// ers
#include <erslib/contrib/json.hpp>


// Basic tests -----------------------------------------------------------------

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


// Object access tests with custom traits --------------------------------------


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
    CHECK_THROWS_AS(std::ignore = view.at("b"), ers::out_of_range_error);
    CHECK_THROWS_AS(std::ignore = view["b"], ers::out_of_range_error);

    const utl::Json number = 5;
    CHECK_THROWS_AS(std::ignore = number.at("a"), ers::conversion_error);
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


// Array access test with custom traits ----------------------------------------


struct deque_traits : utl::default_traits {
    template<class Node>
    using array_type = std::deque<Node>;
};


TEST_CASE("index access on an array") {
    utl::Json json = utl::Array { utl::Json(1), utl::Json(2) };

    CHECK(json[0].as_integral() == 1);
    CHECK(json.at(1).as_integral() == 2);

    json[0] = "x";
    CHECK(json.at(0).as_string() == "x");

    const auto& view = json;
    CHECK(view[1].as_integral() == 2);
    CHECK(view.at(1).as_integral() == 2);
}

TEST_CASE("index access checks the type and the range") {
    utl::Json json = utl::Array { utl::Json(1) };

    CHECK_THROWS_AS(std::ignore = json.at(1), ers::out_of_range_error);

    utl::Json object;
    object["a"] = 1;
    CHECK_THROWS_AS(std::ignore = object[0], ers::conversion_error);
    CHECK_THROWS_AS(std::ignore = object.at(0), ers::conversion_error);

    CHECK_THROWS_AS(json["0"], ers::conversion_error);
}

TEST_CASE("try_at by index doesn't throw") {
    utl::Json json = utl::Array { utl::Json(1) };

    CHECK(json.try_at(0));
    CHECK_FALSE(json.try_at(1));

    if (auto element = json.try_at(0))
        *element = 2;
    CHECK(json[0].as_integral() == 2);

    const utl::Json number = 5;
    CHECK_FALSE(number.try_at(0));
}

TEST_CASE("push_back creates an array from null") {
    utl::Json json;

    json["items"].push_back(1);
    json["items"].push_back("x");

    REQUIRE(json["items"].is_array());
    CHECK(json["items"].as_array().size() == 2);
    CHECK(json["items"][1].as_string() == "x");
}

TEST_CASE("push_back on a non-array throws") {
    utl::Json json = 5;
    CHECK_THROWS_AS(json.push_back(1), ers::conversion_error);
}

TEST_CASE("emplace_back returns the new element") {
    utl::Json json;

    auto& element = json.emplace_back();
    element["a"] = 1;

    CHECK(json[0]["a"].as_integral() == 1);
}

TEST_CASE("push_back of an own element survives reallocation") {
    utl::Json json = utl::Array { utl::Json("first"), utl::Json(2) };

    for (int i = 0; i < 16; ++i)
        json.push_back(json[0]);

    CHECK(json.as_array().size() == 18);
    CHECK(json[17].as_string() == "first");
}

TEST_CASE("deque as the array type") {
    utl::TNode<deque_traits> json;

    json.push_back(1);
    json.emplace_back("x");

    CHECK(json[0].as_integral() == 1);
    CHECK(json.at(1).as_string() == "x");
    CHECK_FALSE(json.try_at(2));
}


// Exotic assignment tests


TEST_CASE("standard containers become arrays") {
    utl::Json numbers = std::vector { 1, 2, 3 };
    REQUIRE(numbers.is_array());
    CHECK(numbers[2].as_integral() == 3);

    utl::Json strings = std::list { "a", "b" };
    CHECK(strings[1].as_string() == "b");

    utl::Json set = std::set<std::string> { "b", "a" };
    CHECK(set[0].as_string() == "a");

    utl::Json flags = std::array { true, false };
    CHECK(flags[0].as_bool());
    CHECK_FALSE(flags[1].as_bool());

    utl::Json bits = std::vector { true, false };
    CHECK(bits[0].as_bool());
}

TEST_CASE("maps with string keys become objects") {
    utl::Json json = std::map<std::string, int> { { "a", 1 }, { "b", 2 } };
    REQUIRE(json.is_object());
    CHECK(json["b"].as_integral() == 2);

    utl::Json nested = std::unordered_map<std::string, std::vector<double>> { { "x", { 1.5, 2.5 } } };
    CHECK(nested["x"][1].as_floating() == 2.5);
}

TEST_CASE("optional becomes a value or null") {
    utl::Json engaged = std::optional { 5 };
    CHECK(engaged.as_integral() == 5);

    utl::Json empty = std::optional<int> {};
    CHECK(empty.is_null());

    utl::Json json;
    json["price"] = ers::optional<i64> {};
    CHECK(json["price"].is_null());
}

TEST_CASE("unsupported types are rejected at compile time") {
    static_assert(!std::constructible_from<utl::Json, std::pair<int, int>>);
    static_assert(!std::constructible_from<utl::Json, std::map<int, int>>);
    static_assert(std::constructible_from<utl::Json, std::span<const utl::Json>>);
}

TEST_CASE("non-const lvalues of alternative types") {
    std::string string = "x";
    utl::Json from_string = string;
    CHECK(from_string.as_string() == "x");
    CHECK(string == "x");

    utl::Object object;
    object.try_emplace("a", 1);
    utl::Json from_object = object;
    CHECK(from_object["a"].as_integral() == 1);
    CHECK(object.size() == 1);
}

TEST_CASE("char is a number") {
    CHECK(utl::Json('a').as_integral() == 'a');
}

TEST_CASE("conversion between instantiations") {
    utl::Json flat;
    flat["a"]["b"] = utl::Array { utl::Json(1), utl::Json("x") };
    flat["c"] = 1.5;

    utl::TNode<plain_map_traits> plain = flat;
    CHECK(plain["a"]["b"][1].as_string() == "x");
    CHECK(plain["c"].as_floating() == 1.5);

    utl::TNode<custom_key_traits> custom = plain;
    CHECK(custom.contains("a"));

    utl::TNode<deque_traits> deque = std::move(custom);
    CHECK(deque["a"]["b"][0].as_integral() == 1);

    flat = deque;
    CHECK(flat["a"]["b"][1].as_string() == "x");
}

TEST_CASE("moving a container moves its strings") {
    std::vector source { std::string(64, 'x') };

    utl::Json json = std::move(source);
    CHECK(json[0].as_string().size() == 64);
}

TEST_CASE("expected is not treated as optional") {
    static_assert(utl::impl::json_detail::classify<std::expected<int, std::string>, utl::Json>()
        == utl::impl::json_detail::ESourceKind::None);
}


// I/O tests

struct custom_string_traits : utl::default_traits {
    using string_type = test_key;
};


TEST_CASE("parse scalars and nesting") {
    CHECK(utl::from_string("null").is_null());
    CHECK(utl::from_string(" true ").as_bool());
    CHECK(utl::from_string("-42").as_integral() == -42);
    CHECK(utl::from_string("1.5e2").as_floating() == 150.0);
    CHECK(utl::from_string(R"("x")").as_string() == "x");

    const auto json = utl::from_string(R"({ "a": [1, {"b": null}], "c": "d" })");
    CHECK(json["a"][1].contains("b"));
    CHECK(json["c"].as_string() == "d");
}

TEST_CASE("parse escapes and unicode") {
    CHECK(utl::from_string(R"("a\"b\\c\n\t\/")").as_string() == "a\"b\\c\n\t/");
    CHECK(utl::from_string(R"("\u00e9")").as_string() == "\xC3\xA9");
    CHECK(utl::from_string(R"("\ud83d\ude00")").as_string() == "\xF0\x9F\x98\x80");
}

TEST_CASE("parse rejects invalid input") {
    for (std::string_view input : {
             "", "   ", "01", "1.", "-", "-inf", "+1", ".5", "[1,]", "{\"a\":1,}", "[1 2]", "{\"a\" 1}",
             "tru", "nul", "\"abc", "\"a\x01\"", R"("\x")", R"("\ud83d")", R"("\ude00")", R"("\ud83d\u0041")",
             "1 2", "{} x"
         }) {
        CAPTURE(input);
        CHECK_THROWS_AS(static_cast<void>(utl::from_string(input)), ers::parse_error);
    }
}

TEST_CASE("integers and floats are kept apart") {
    CHECK(utl::from_string("9223372036854775807").is_integral());
    CHECK(utl::from_string("-9223372036854775808").is_integral());
    CHECK_THROWS_AS(static_cast<void>(utl::from_string("9223372036854775808")), ers::parse_error);
    CHECK_THROWS_AS(static_cast<void>(utl::from_string("-9223372036854775809")), ers::parse_error);

    CHECK(utl::from_string("1").is_integral());
    CHECK(utl::from_string("1.0").is_floating());
    CHECK(utl::from_string("1e0").is_floating());
}

TEST_CASE("recursion limit") {
    const auto nested = [](std::size_t depth) {
        return std::string(depth, '[') + std::string(depth, ']');
    };

    CHECK_NOTHROW(static_cast<void>(utl::from_string(nested(1000))));
    CHECK_THROWS_AS(static_cast<void>(utl::from_string(nested(1001))), ers::parse_error);
    CHECK_THROWS_AS(static_cast<void>(utl::from_string(nested(11), 10)), ers::parse_error);
}

TEST_CASE("serialize numbers") {
    CHECK(utl::Json(2.0).to_string(utl::EFormat::Minimized) == "2.0");
    CHECK(utl::Json(1.5).to_string(utl::EFormat::Minimized) == "1.5");
    CHECK(utl::Json(1e300).to_string(utl::EFormat::Minimized) == "1e+300");
    CHECK(utl::Json(std::numeric_limits<double>::quiet_NaN()).to_string(utl::EFormat::Minimized) == "null");
    CHECK(utl::Json(std::numeric_limits<i64>::min()).to_string(utl::EFormat::Minimized) == "-9223372036854775808");
}

TEST_CASE("serialize escapes strings and keys") {
    utl::TNode<plain_map_traits> json;
    json["k\"ey"] = std::string("a\x01\"\\\n");

    CHECK(json.to_string(utl::EFormat::Minimized) == R"({"k\"ey":"a\u0001\"\\\n"})");
}

TEST_CASE("pretty format") {
    const auto json = utl::from_string<utl::TNode<plain_map_traits>>(R"({"b":[true,null],"a":1,"e":{},"f":[]})");

    CHECK(json.to_string() ==
        "{\n"
        "    \"a\": 1,\n"
        "    \"b\": [\n"
        "        true,\n"
        "        null\n"
        "    ],\n"
        "    \"e\": {},\n"
        "    \"f\": []\n"
        "}");
}

TEST_CASE("round trip") {
    constexpr std::string_view source = R"({"a":[1,2.5,"x\u0001",true,null,{"b":-3}],"c":"\u00e9"})";

    const auto first = utl::from_string<utl::TNode<plain_map_traits>>(source);
    const auto text = first.to_string(utl::EFormat::Minimized);
    const auto second = utl::from_string<utl::TNode<plain_map_traits>>(text);

    CHECK(second.to_string(utl::EFormat::Minimized) == text);
    CHECK(second["a"][1].is_floating());
    CHECK(second["c"].as_string() == "\xC3\xA9");
}

TEST_CASE("custom string type goes through string_adaptor") {
    const auto json = utl::from_string<utl::TNode<custom_string_traits>>(R"({"a":"x"})");

    CHECK(json["a"].as_string().value == "x");
    CHECK(json.to_string(utl::EFormat::Minimized) == R"({"a":"x"})");
}

TEST_CASE("try_from_string and files") {
    CHECK_FALSE(utl::try_from_string("{"));
    CHECK(utl::try_from_string("{}"));

    const auto path = std::filesystem::temp_directory_path() / "erslib_json_test" / "file.json";

    utl::Json json;
    json["a"] = 1;
    json.to_file(path);

    CHECK(utl::from_file(path)["a"].as_integral() == 1);
    CHECK_FALSE(utl::try_from_file(path.parent_path() / "missing.json"));

    std::filesystem::remove_all(path.parent_path());
}

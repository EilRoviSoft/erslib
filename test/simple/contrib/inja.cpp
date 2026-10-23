#ifdef _HAS_INJA

// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>
#include <concepts>
#include <iterator>
#include <string>
#include <vector>

// ers
#include <erslib/contrib/inja.hpp>

// test
#include "support/test_case.hpp"


// Facade tests ----------------------------------------------------------------

using utl::operator""_json;


ERS_TEST_CASE("binding", "[contrib][compile_time]") {
    static_assert(std::same_as<inja::json, utl::InjaJson>);
    static_assert(std::bidirectional_iterator<utl::InjaJson::iterator>);
    static_assert(std::bidirectional_iterator<utl::InjaJson::const_iterator>);
}

ERS_TEST_CASE("getters", "[contrib]") {
    const utl::InjaJson integral = 5;
    const utl::InjaJson floating = 2.5;

    CHECK(integral.get<const utl::InjaJson::number_integer_t>() == 5);
    CHECK(integral.get<utl::InjaJson::number_float_t>() == 5.0);
    CHECK(floating.get<int>() == 2);
    CHECK(integral.is_integral());
    CHECK(floating.is_floating());

    CHECK(utl::InjaJson("x").get<std::string>() == "x");
    CHECK(utl::InjaJson("x").get_ref<const utl::InjaJson::string_t&>() == "x");
    CHECK(utl::InjaJson(true).get<bool>());

    CHECK_THROWS(static_cast<void>(utl::InjaJson("x").get<int>()));
    CHECK_THROWS(static_cast<void>(utl::InjaJson(1).get<std::string>()));
}

ERS_TEST_CASE("size", "[contrib]") {
    CHECK(utl::InjaJson().size() == 0);
    CHECK(utl::InjaJson().empty());
    CHECK(utl::InjaJson(1).size() == 1);
    CHECK_FALSE(utl::InjaJson(0).empty());
    CHECK(utl::InjaJson("").empty());
    CHECK_FALSE(utl::InjaJson("x").empty());
    CHECK(utl::InjaJson::parse("[1,2,3]").size() == 3);
}

ERS_TEST_CASE("clear", "[contrib]") {
    auto json = utl::InjaJson::parse(R"({"a":1,"b":"x","c":[1]})");

    json["a"].clear();
    json["b"].clear();
    json["c"].clear();

    CHECK(json.dump() == R"({"a":0,"b":"","c":[]})");
}

ERS_TEST_CASE("iteration", "[contrib]") {
    auto object = utl::InjaJson::parse(R"({"b":1,"a":2})");

    std::vector<std::string> keys;
    for (auto it = object.begin(); it != object.end(); ++it) {
        keys.push_back(it.key());
        CHECK(it.value().is_integral());
    }
    CHECK(keys == std::vector<std::string> { "a", "b" });

    const utl::InjaJson scalar = 7;
    CHECK(std::distance(scalar.begin(), scalar.end()) == 1);
    CHECK(*scalar.begin() == 7);

    const utl::InjaJson null;
    CHECK(null.begin() == null.end());

    CHECK(object.find("a") != object.end());
    CHECK(object.find("z") == object.end());
    CHECK(scalar.find("a") == scalar.end());

    CHECK_THROWS(static_cast<void>(scalar.begin().key()));
}

ERS_TEST_CASE("front and back", "[contrib]") {
    const auto array = utl::InjaJson::parse("[1,2,3]");
    CHECK(array.front() == 1);
    CHECK(array.back() == 3);

    CHECK_THROWS(static_cast<void>(utl::InjaJson().front()));
    CHECK_THROWS(static_cast<void>(utl::InjaJson::parse("[]").back()));
}

ERS_TEST_CASE("comparison", "[contrib]") {
    CHECK(utl::InjaJson(1) == utl::InjaJson(1.0));
    CHECK(utl::InjaJson(1) < utl::InjaJson(1.5));
    CHECK(utl::InjaJson(0) == 0);
    CHECK(utl::InjaJson(0.0) == 0);
    CHECK(utl::InjaJson("a") < utl::InjaJson("b"));
    CHECK(utl::InjaJson() < utl::InjaJson(false));
    CHECK(utl::InjaJson(true) < utl::InjaJson(0));
    CHECK(utl::InjaJson(1) < utl::InjaJson("1"));
    CHECK(utl::InjaJson(1) != utl::InjaJson("1"));

    CHECK(utl::InjaJson::parse(R"({"a":[1,{"b":2}]})") == utl::InjaJson::parse(R"({"a":[1.0,{"b":2}]})"));
    CHECK(utl::InjaJson::parse("[1,2]") < utl::InjaJson::parse("[1,3]"));
}

ERS_TEST_CASE("sort", "[contrib]") {
    auto array = utl::InjaJson::parse(R"([3,"b",1.5,null,true,"a"])");
    std::sort(array.begin(), array.end());
    CHECK(array.dump() == R"([null,true,1.5,3,"a","b"])");
}


// Rendering tests -------------------------------------------------------------

ERS_TEST_CASE("variables", "[contrib]") {
    inja::Environment env;

    utl::Json data;
    data["name"] = "Ann";
    data["count"] = 3;
    data["price"] = 2.0;
    data["flag"] = true;
    data["items"] = utl::Array { utl::Json(1), utl::Json(2) };

    CHECK(env.render("{{ name }}: {{ count }} x {{ price }} {{ flag }} {{ items }}", data)
        == "Ann: 3 x 2.0 true [1,2]");
}

ERS_TEST_CASE("object loops", "[contrib]") {
    inja::Environment env;

    const auto data = utl::from_string<utl::InjaJson>(R"({"map":{"b":2,"a":1,"c":3}})");

    CHECK(env.render("{% for key, value in map %}{{ key }}={{ value }}{% if not loop.is_last %},{% endif %}{% endfor %}", data)
        == "a=1,b=2,c=3");
}

ERS_TEST_CASE("nested loops", "[contrib]") {
    inja::Environment env;

    const auto data = R"({"rows":[[1,2],[3]],"map":{"x":{"y":1}}})"_json;

    CHECK(env.render(
        "{% for row in rows %}{% for cell in row %}{{ loop.parent.index }}.{{ loop.index }}={{ cell }};{% endfor %}{% endfor %}",
        data) == "0.0=1;0.1=2;1.0=3;");

    CHECK(env.render(
        "{% for k, v in map %}{% for k2, v2 in v %}{% set total = loop.parent.index1 + loop.index1 %}{{ k }}{{ k2 }}{{ total }}{% endfor %}{{ loop.index }}{% endfor %}"
        ,
        data) == "xy20");
}

ERS_TEST_CASE("set and functions", "[contrib]") {
    inja::Environment env;

    const auto data = R"({"a":7,"b":2,"f":0.5,"list":[3,1,2],"text":"Hello"})"_json;

    CHECK(env.render("{{ a + b }} {{ a * f }} {{ a / b }} {{ a % b }}", data) == "9 3.5 3.5 1");
    CHECK(env.render("{% set x = a - b %}{{ x }}", data) == "5");
    CHECK(env.render("{% set obj.inner = 4 %}{{ obj.inner }}", data) == "4");
    CHECK(env.render("{{ sort(list) }} {{ max(list) }} {{ min(list) }} {{ length(list) }} {{ first(list) }} {{ last(list) }}", data)
        == "[1,2,3] 3 1 3 3 2");
    CHECK(env.render("{{ upper(text) }} {{ length(text) }} {{ join(list, \"-\") }} {{ round(f, 0) }}", data)
        == "HELLO 5 3-1-2 1");
    CHECK(env.render("{% if 2 in list %}yes{% endif %} {{ at(list, 1) }} {{ range(3) }}", data) == "yes 1 [0,1,2]");
}

ERS_TEST_CASE("truthiness", "[contrib]") {
    inja::Environment env;

    const auto data = R"({"empty":"","zero":0,"fzero":0.0,"none":null,"list":[],"text":"x"})"_json;

    CHECK(env.render("{% if empty %}T{% else %}F{% endif %}", data) == "F");
    CHECK(env.render("{% if zero %}T{% else %}F{% endif %}", data) == "F");
    CHECK(env.render("{% if fzero %}T{% else %}F{% endif %}", data) == "F");
    CHECK(env.render("{% if none %}T{% else %}F{% endif %}", data) == "F");
    CHECK(env.render("{% if list %}T{% else %}F{% endif %}", data) == "F");
    CHECK(env.render("{% if text %}T{% else %}F{% endif %}", data) == "T");
}

ERS_TEST_CASE("exists and default", "[contrib]") {
    inja::Environment env;

    const auto data = R"({"user":{"name":"Ann"}})"_json;

    CHECK(env.render("{{ exists(\"user.name\") }} {{ exists(\"user.age\") }}", data) == "true false");
    CHECK(env.render("{{ existsIn(user, \"name\") }}", data) == "true");
    CHECK(env.render("{{ default(user.age, 18) }}", data) == "18");
    CHECK_THROWS_AS(static_cast<void>(env.render("{{ user.age }}", data)), inja::RenderError);
}

ERS_TEST_CASE("autoescape", "[contrib]") {
    inja::Environment env;
    env.set_html_autoescape(true);

    const auto data = R"({"text":"<b>\"Tom\" & 'Jerry'</b>"})"_json;

    CHECK(env.render("{{ text }}", data) == "&lt;b&gt;&quot;Tom&quot; &amp; &apos;Jerry&apos;&lt;/b&gt;");
}

ERS_TEST_CASE("callbacks", "[contrib]") {
    inja::Environment env;

    env.add_callback("money", 1, [](inja::Arguments& args) {
        return std::to_string(args.at(0)->get<int>()) + " RUB";
    });

    CHECK(env.render("{{ money(price) }}", R"({"price":150})"_json) == "150 RUB");
}

#endif

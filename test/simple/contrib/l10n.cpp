// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/contrib/l10n.hpp>
#include <erslib/core/localization.hpp>

// test
#include "support/test_case.hpp"
#include "support/temp_tree.hpp"


using test::TempTree;


namespace {
    using pairs_t = std::vector<std::pair<std::string, std::string>>;

    // object order isn't kept, so the result is sorted
    ers::Result<pairs_t> flatten(std::string_view json) {
        auto entries = ERS_TRY(utl::flatten_l10n(utl::from_string(json)));;

        pairs_t out;

        for (auto& [key, text] : entries)
            out.emplace_back(std::move(key), std::move(text));

        std::ranges::sort(out);
        return out;
    }
}


ERS_TEST_CASE("nesting", "[contrib]") {
    auto entries = flatten(R"({
        "black": { "hex": "#000000", "name@ru": "чёрный" },
        "delivery": ["Бесплатно.", "Сроки уточнит менеджер."],
        "brand": "Маленькая Леди"
    })");

    REQUIRE(entries);
    CHECK(*entries == pairs_t {
            { "black.hex", "#000000" },
        { "black.name@ru", "чёрный" },
        { "brand", "Маленькая Леди" },
        { "delivery", "Бесплатно.\nСроки уточнит менеджер." },
        });

    CHECK(flatten("{}")->empty());
}

ERS_TEST_CASE("value types", "[contrib]") {
    CHECK_FALSE(flatten(R"("text")"));
    CHECK_FALSE(flatten(R"(["a"])"));

    CHECK_FALSE(flatten(R"({ "a": 1 })"));
    CHECK_FALSE(flatten(R"({ "a": 1.5 })"));
    CHECK_FALSE(flatten(R"({ "a": true })"));
    CHECK_FALSE(flatten(R"({ "a": null })"));
    CHECK_FALSE(flatten(R"({ "a": ["x", 1] })"));
    CHECK_FALSE(flatten(R"({ "a": [["x"]] })"));

    CHECK_FALSE(flatten(R"({ "a.b": "x" })"));
    CHECK_FALSE(flatten(R"({ "": "x" })"));
    CHECK_FALSE(flatten(R"({ "a": { "": "x" } })"));
}

ERS_TEST_CASE("loader", "[contrib]") {
    const auto loader = utl::l10n_json_loader();

    CHECK(loader.suffix == ".l10n.json");
    CHECK(loader.parse(R"({ "a": "x" })"));
    CHECK_FALSE(loader.parse("{ not json"));
}

ERS_TEST_CASE("table", "[contrib][fs]") {
    TempTree tree;
    tree.write("ru/mail/new_order.txt", "Новый заказ");
    tree.write("_common/colors.l10n.json", R"({
        "black": { "hex": "#000000", "name@ru": "чёрный", "name@en": "black" },
        "milky": { "hex": "#F8F9FA", "name@ru": "молочный" }
    })");
    tree.write("en/ui/buttons.l10n.json", R"({ "cart": { "add": "Add to cart" } })");

    tree.write("ru/ui/config.json", R"({ "a": 1 })");

    ers::LocaleTable table({
        .fallback_locale = "ru",
        .bundles         = { utl::l10n_json_loader() },
    });

    REQUIRE(table.reload(tree.root()));

    CHECK(table.config()->locales == std::vector<std::string> { "en", "ru" });

    CHECK(table["en"]["colors"]["black.name"] == "black");
    CHECK(table["en"]["colors"]["milky.name"] == "молочный");
    CHECK(table["ru"]["colors"]["milky.hex"] == "#F8F9FA");
    CHECK(table["en"]["colors"].children() == std::vector<std::string_view> { "black", "milky" });

    CHECK(table["en"]["ui"]["buttons.cart.add"] == "Add to cart");
    CHECK(table["ru"]["ui"]["json.config"] == R"({ "a": 1 })");
}

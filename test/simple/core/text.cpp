// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/text/language_layout.hpp>
#include <erslib/core/text/utf8.hpp>

// test
#include "support/test_case.hpp"


namespace {
    constexpr std::string append_one(char32_t code) {
        std::string out;
        ers::utf8::append(out, code);
        return out;
    }
}


// "я" is D1 8F, "€" is E2 82 AC, "😀" is F0 9F 98 80

static_assert(ers::utf8::truncate("x\xD1\x8F", 2) == "x");
static_assert(ers::utf8::truncate("x\xD1\x8F", 3) == "x\xD1\x8F");
static_assert(ers::utf8::is_valid("x\xD1\x8F\xE2\x82\xAC\xF0\x9F\x98\x80"));
static_assert(!ers::utf8::is_valid("x\xD1"));
static_assert(append_one(U'й') == "\xD0\xB9");


ERS_TEST_CASE("utf8 truncate", "[core]") {
    REQUIRE(ers::utf8::truncate("", 0).empty());
    REQUIRE(ers::utf8::truncate("abc", 0).empty());
    REQUIRE(ers::utf8::truncate("abc", 3) == "abc");
    REQUIRE(ers::utf8::truncate("abc", 10) == "abc");

    REQUIRE(ers::utf8::truncate("юбка", 4) == "юб");
    REQUIRE(ers::utf8::truncate("юбка", 3) == "ю");
    REQUIRE(ers::utf8::truncate("юбка", 1).empty());

    REQUIRE(ers::utf8::truncate("a€", 3) == "a");
    REQUIRE(ers::utf8::truncate("a€", 4) == "a€");
    REQUIRE(ers::utf8::truncate("a😀b", 4) == "a");
    REQUIRE(ers::utf8::truncate("a😀b", 5) == "a😀");

    for (size_t max = 0; max <= 20; ++max) {
        const auto cut = ers::utf8::truncate("красное платье", max);

        CHECK(cut.size() <= max);
        CHECK(ers::utf8::is_valid(cut));
    }
}

ERS_TEST_CASE("utf8 is_valid", "[core]") {
    REQUIRE(ers::utf8::is_valid(""));
    REQUIRE(ers::utf8::is_valid("FSL-571 платье ё € 😀"));

    REQUIRE_FALSE(ers::utf8::is_valid("\x80"));
    REQUIRE_FALSE(ers::utf8::is_valid("x\xD1"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xE2\x82"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xD1x"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xC0\x80"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xE0\x80\xAF"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xED\xA0\x80"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xF4\x90\x80\x80"));
    REQUIRE_FALSE(ers::utf8::is_valid("\xFF"));
}

ERS_TEST_CASE("utf8 append", "[core]") {
    REQUIRE(append_one(U'a') == "a");
    REQUIRE(append_one(U'ё') == "ё");
    REQUIRE(append_one(U'€') == "€");
    REQUIRE(append_one(U'😀') == "😀");

    std::string out = "x";
    ers::utf8::append(out, U'я');
    REQUIRE(out == "xя");
}

ERS_TEST_CASE("keyboard layout", "[core]") {
    REQUIRE(ers::text::ru_to_en("").empty());

    REQUIRE(ers::text::ru_to_en("gkfnmt") == "платье");
    REQUIRE(ers::text::ru_to_en(",h.rb") == "брюки");
    REQUIRE(ers::text::ru_to_en(".,rf") == "юбка");
    REQUIRE(ers::text::ru_to_en("`krf") == "ёлка");
    REQUIRE(ers::text::ru_to_en("{jkjlysq") == "Холодный");
    REQUIRE(ers::text::ru_to_en("RFHLBUFY") == "КАРДИГАН");

    REQUIRE(ers::text::ru_to_en("gkfnmt 128") == "платье 128");
    REQUIRE(ers::text::ru_to_en("юбка") == "юбка");
    REQUIRE(ers::text::ru_to_en("gkfnmt-юбка") == "платье-юбка");

    const auto all = ers::text::ru_to_en("qwertyuiop[]asdfghjkl;'zxcvbnm,.`");
    REQUIRE(all == "йцукенгшщзхъфывапролджэячсмитьбюё");
    REQUIRE(ers::utf8::is_valid(all));
}

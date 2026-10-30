// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <random>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/algorithm.hpp>

// test
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    using strings = std::vector<std::string>;


    strings split(std::string_view line) {
        auto result = ers::split_args(line);

        return result ? *result : strings { "<error>" };
    }
}


// Splitting -------------------------------------------------------------------

ERS_TEST_CASE("plain", "[core]") {
    REQUIRE(split("") == strings{});
    REQUIRE(split("   ") == strings{});
    REQUIRE(split("a b  c") == strings{ "a", "b", "c" });
    REQUIRE(split("  a\tb\n") == strings{ "a", "b" });
}

ERS_TEST_CASE("quotes", "[core]") {
    REQUIRE(split("a 'b c' \"d e\"") == strings{ "a", "b c", "d e" });
    REQUIRE(split("a\"b c\"d") == strings{ "ab cd" });
    REQUIRE(split("'' \"\"") == strings{ "", "" });
    REQUIRE(split("'a\"b'") == strings{ "a\"b" });
    REQUIRE(split("\"a'b\"") == strings{ "a'b" });
}

ERS_TEST_CASE("escape", "[core]") {
    REQUIRE(split("a\\ b") == strings{ "a b" });
    REQUIRE(split("a\\'b") == strings{ "a'b" });
    REQUIRE(split("a\\\"b") == strings{ "a\"b" });
    REQUIRE(split("a\\\\b") == strings{ "a\\b" });
    REQUIRE(split("\\ ") == strings{ " " });
}

ERS_TEST_CASE("inside", "[core]") {
    REQUIRE(split("\"a\\\"b\"") == strings{ "a\"b" });
    REQUIRE(split("\"a\\\\b\"") == strings{ "a\\b" });
    REQUIRE(split("\"a\\nb\"") == strings{ "a\\nb" });
    REQUIRE(split("'a\\nb'") == strings{ "a\\nb" });
    REQUIRE(split("'it'\\''s'") == strings{ "it's" });
}

ERS_TEST_CASE("broken", "[core]") {
    REQUIRE(!ers::split_args("'abc"));
    REQUIRE(!ers::split_args("\"abc"));
    REQUIRE(!ers::split_args("abc\\"));
    REQUIRE(!ers::split_args("\"abc\\"));
}


// Quoting ---------------------------------------------------------------------

ERS_TEST_CASE("quote", "[core]") {
    REQUIRE(ers::quote_arg("file.mp4") == "file.mp4");
    REQUIRE(ers::quote_arg("/tmp/a-b_c.d") == "/tmp/a-b_c.d");
    REQUIRE(ers::quote_arg("") == "''");
    REQUIRE(ers::quote_arg("a b") == "'a b'");
    REQUIRE(ers::quote_arg("it's") == "'it'\\''s'");
    REQUIRE(ers::quote_arg("$HOME;rm -rf /") == "'$HOME;rm -rf /'");
}

ERS_TEST_CASE("embed", "[core]") {
    strings tricky = {
        "plain", "", " ", "a b", "it's", "say \"hi\"", "both ' and \"", "back\\slash", "\\", "'", "\"", "''", "\\'",
        "$HOME", "`x`", "a;b", "line\nbreak", "tab\there", "C:\\dir\\file.mp4", "-i", "*.mp4",
    };

    REQUIRE(split(ers::join_args(tricky)) == tricky);

    for (const auto& one : tricky)
        REQUIRE(split(ers::quote_arg(one)) == strings{ one });
}

ERS_TEST_CASE("fuzz", "[core]") {
    const std::string alphabet = "ab '\"\\\n\t$;-.";

    std::mt19937 rng(12345);
    std::uniform_int_distribution<size_t> letter(0, alphabet.size() - 1);
    std::uniform_int_distribution<size_t> length(0, 12);
    std::uniform_int_distribution<size_t> count(0, 5);

    for (int round = 0; round < 20000; ++round) {
        strings args(count(rng));

        for (auto& arg : args) {
            size_t size = length(rng);

            for (size_t i = 0; i < size; ++i)
                arg.push_back(alphabet[letter(rng)]);
        }

        auto back = ers::split_args(ers::join_args(args));

        REQUIRE(back.has_value());
        REQUIRE(*back == args);
    }
}

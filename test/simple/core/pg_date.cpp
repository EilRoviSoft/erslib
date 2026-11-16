// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <string_view>

// ers
#include <erslib/core/convert/pg_time.hpp>

// test
#include "support/test_case.hpp"


using namespace std::chrono_literals;


namespace {
    ers::date_t parsed(std::string_view what) {
        auto r = ers::date_from_iso8601(what);
        REQUIRE(r.has_value());
        return *r;
    }

    bool refused(std::string_view what) {
        return !ers::date_from_iso8601(what).has_value();
    }
}


ERS_TEST_CASE("date: values", "[core]") {
    CHECK(parsed("2030-01-10") == ers::date_t(2030y / 1 / 10));
    CHECK(parsed("1970-01-01") == ers::date_t{});
    CHECK(parsed("2000-02-29") == ers::date_t(2000y / 2 / 29));

    CHECK(ers::date_to_iso8601(ers::date_t{}) == "1970-01-01");
    CHECK(ers::date_to_iso8601(ers::date_t(2030y / 1 / 10)) == "2030-01-10");
}

ERS_TEST_CASE("date: round trip", "[core]") {
    for (std::string_view text : {
        "2030-01-10",
        "2000-01-01",
        "2000-02-29",
        "2026-12-31",
        "0001-01-01",
        "9999-12-31",
        "10000-01-01",
        "0001-12-31 BC",
        "0001-01-01 BC",
        "4713-11-24 BC",
    }) {
        CAPTURE(text);
        CHECK(ers::date_to_iso8601(parsed(text)) == text);
    }
}

ERS_TEST_CASE("date: BC", "[core]") {
    // 1 BC is the year 0 of the proleptic calendar and a leap year
    CHECK(parsed("0001-01-01 BC") == ers::date_t(0y / 1 / 1));
    CHECK(parsed("0001-12-31 BC") + std::chrono::days{1} == parsed("0001-01-01"));
    CHECK(parsed("0001-01-01") - parsed("0001-01-01 BC") == std::chrono::days{366});

    // there is no year zero
    CHECK(refused("0000-01-01"));
    CHECK(refused("0000-01-01 BC"));

    CHECK(refused("0001-01-01 AD"));
    CHECK(refused("0001-01-01 bc"));
    CHECK(refused("0001-01-01  BC"));
    CHECK(refused("0001-01-01 BCE"));
}

ERS_TEST_CASE("date: leap years", "[core]") {
    CHECK(ers::date_from_iso8601("2024-02-29").has_value());
    CHECK(ers::date_from_iso8601("2000-02-29").has_value());

    CHECK(refused("2023-02-29"));
    CHECK(refused("1900-02-29"));
}

ERS_TEST_CASE("date: malformed", "[core]") {
    for (std::string_view text : {
        "",
        "garbage",
        "2030-02-30",
        "2030-13-01",
        "2030-00-10",
        "2030-01-00",
        "2030-01-32",
        "2030-1-1",
        "2030-01-1",
        "203-01-10",
        "123456-01-10",
        "10.01.2030",
        "2030/01/10",
        "2030-01-10 ",
        " 2030-01-10",
        "2030-01-10x",
        "2030-01-10T00:00:00",
        "2030-01-10 00:00:00+00",
        "infinity",
        "-infinity",
    }) {
        CAPTURE(text);
        CHECK(refused(text));
    }
}

ERS_TEST_CASE("date: arithmetic", "[core]") {
    CHECK(parsed("2030-01-31") + std::chrono::days{1} == parsed("2030-02-01"));
    CHECK(parsed("2024-02-28") + std::chrono::days{1} == parsed("2024-02-29"));
    CHECK(parsed("2023-02-28") + std::chrono::days{1} == parsed("2023-03-01"));
    CHECK(parsed("2030-01-10") < parsed("2030-01-11"));

    CHECK(ers::date_to_iso8601(parsed("2026-12-31") + std::chrono::days{1}) == "2027-01-01");
}

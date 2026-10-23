// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <string_view>

// ers
#include <erslib/core/convert/pg_time.hpp>

// test
#include "support/tags.hpp"


using namespace std::chrono_literals;


namespace {
    ers::timestamp_t parsed(std::string_view what) {
        auto r = ers::timestamp_from_iso8601(what);
        REQUIRE(r.has_value());
        return *r;
    }
}


TEST_CASE("round_trip", TEST_TAGS) {
    const auto now = ers::floor_to_pg_precision(ers::clock_t::now());
    CHECK(parsed(ers::timestamp_to_iso8601(now)) == now);
}

TEST_CASE("offsets", TEST_TAGS) {
    const auto utc = parsed("2026-09-17 12:00:00+00");

    CHECK(parsed("2026-09-17 15:00:00+03") == utc);
    CHECK(parsed("2026-09-17 17:30:00+05:30") == utc);
    CHECK(parsed("2026-09-17 06:30:00-05:30") == utc);
    CHECK(parsed("2026-09-17T12:00:00Z") == utc);

    CHECK(ers::timestamp_to_iso8601(utc) == "2026-09-17 12:00:00.000000+00");
}

TEST_CASE("fractions", TEST_TAGS) {
    CHECK(parsed("2026-09-17 12:00:00.5+00")
        == parsed("2026-09-17 12:00:00+00") + 500ms);

    CHECK(ers::timestamp_to_iso8601(parsed("2026-09-17 12:00:00.123456+00"))
        == "2026-09-17 12:00:00.123456+00");
}

TEST_CASE("infinities", TEST_TAGS) {
    CHECK(parsed("infinity") == ers::timestamp_t::max());
    CHECK(parsed("-infinity") == ers::timestamp_t::min());

    CHECK(ers::timestamp_to_iso8601(ers::timestamp_t::max()) == "infinity");
    CHECK(ers::timestamp_to_iso8601(ers::timestamp_t::min()) == "-infinity");
}

TEST_CASE("malformed", TEST_TAGS) {
    // a naive TIMESTAMP column prints no offset, and we refuse to assume one
    CHECK_FALSE(ers::timestamp_from_iso8601("2026-09-17 12:00:00").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("2026-02-30 00:00:00+00").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("2026-09-17 25:00:00+00").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("2026-09-17 12:00:00+00 junk").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("2026-09-17 12:00:00.1234567+00").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("garbage").has_value());
    CHECK_FALSE(ers::timestamp_from_iso8601("").has_value());

    // outside what system_clock::duration can hold (1677..2262 on libstdc++)
    CHECK_FALSE(ers::timestamp_from_iso8601("1600-01-01 00:00:00+00").has_value());
}

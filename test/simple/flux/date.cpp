// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>
#include <erslib/flux/traits/date.hpp>

#ifdef ERSLIB_HAS_REFLECTION
#   include <erslib/flux/reflect.hpp>
#endif

// test
#include "support/test_case.hpp"


using namespace std::chrono_literals;


// Basic tests -----------------------------------------------------------------

ERS_TEST_CASE("date: sql type", "[flux][compile_time]") {
    static_assert(flux::sql_type<ers::date_t>::name == "DATE");
    static_assert(!flux::sql_type<ers::date_t>::nullable);

    static_assert(flux::sql_type<ers::maybe<ers::date_t>>::name == "DATE");
    static_assert(flux::sql_type<ers::maybe<ers::date_t>>::nullable);
}

ERS_TEST_CASE("date: sql text", "[flux]") {
    CHECK(flux::sql_value<ers::date_t>::to_text(ers::date_t(2030y / 1 / 10)) == "2030-01-10");
    CHECK(flux::sql_value<ers::date_t>::to_text(ers::date_t{}) == "1970-01-01");
    CHECK(flux::sql_value<ers::date_t>::to_text(ers::date_t(2000y / 2 / 29)) == "2000-02-29");
}


// Reflection ------------------------------------------------------------------

#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct DateSchema : flux::reflect::Schema<"public"> {};

    struct Deadline {
        ers::date_t day;
        ers::maybe<ers::date_t> due;
    };
}

template<>
struct flux::reflect::Declaration<Deadline> : Table<"deadlines", DateSchema> {
    Unique<"day"> deadlines_day_unique;
};


ERS_TEST_CASE("date: column", "[flux]") {
    const auto ddl = flux::ddl::create_table<Deadline>();

    CHECK(ddl.contains("day DATE NOT NULL"));

    CHECK(ddl.contains("due DATE"));
    CHECK_FALSE(ddl.contains("due DATE NOT NULL"));
}

#endif

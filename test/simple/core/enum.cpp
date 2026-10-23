// catch2
#include <catch2/catch_test_macros.hpp>

#ifdef ERSLIB_HAS_REFLECTION

// ers
#include <erslib/core/enum.hpp>

// test
#include "support/test_case.hpp"


using namespace ers::enum_utils;


namespace {
    enum class Color {
        Red [[=as_is]],
        DarkGreen [[=custom<"forest">]],
        LightBlue,
    };

    enum class Status {
        NotStarted,
        InProgress,
        Done,
    };

    enum class Direction {
        NorthWest,
        SouthEast,
    };

    enum class Weekday {
        MondayStart,
        FridayEnd,
    };

    enum class Priority {
        VeryHigh,
        Low,
    };
}

template<>
struct ers::enum_traits<Status> {
    using from = pascal_case_t;
    using to = upper_snake_case_t;
};

template<>
struct ers::enum_traits<Direction> {
    using from = pascal_case_t;
    using to = kebab_case_t;
};

template<>
struct ers::enum_traits<Weekday> {
    using from = pascal_case_t;
    using to = camel_case_t;
};

template<>
struct ers::enum_traits<Priority> {
    using from = pascal_case_t;
    using to = snake_case_t;
};


ERS_TEST_CASE("pascal", "[core]") {
    CHECK(to_string(Color::Red) == "Red");
    CHECK(to_string(Color::DarkGreen) == "forest");
    CHECK(to_string(Color::LightBlue) == "LightBlue");

    CHECK(from_string<Color>("Red") == Color::Red);
    CHECK(from_string<Color>("forest") == Color::DarkGreen);
    CHECK(from_string<Color>("LightBlue") == Color::LightBlue);
    CHECK_FALSE(from_string<Color>("nope").has_value());
}

ERS_TEST_CASE("upper snake", "[core]") {
    CHECK(to_string(Status::NotStarted) == "NOT_STARTED");
    CHECK(to_string(Status::InProgress) == "IN_PROGRESS");
    CHECK(to_string(Status::Done) == "DONE");

    CHECK(from_string<Status>("IN_PROGRESS") == Status::InProgress);
    CHECK_FALSE(from_string<Status>("InProgress").has_value());
}

ERS_TEST_CASE("kebab and camel", "[core]") {
    CHECK(to_string(Direction::NorthWest) == "north-west");
    CHECK(to_string(Direction::SouthEast) == "south-east");
    CHECK(from_string<Direction>("north-west") == Direction::NorthWest);

    CHECK(to_string(Weekday::MondayStart) == "mondayStart");
    CHECK(to_string(Weekday::FridayEnd) == "fridayEnd");
    CHECK(from_string<Weekday>("fridayEnd") == Weekday::FridayEnd);
}

ERS_TEST_CASE("snake", "[core]") {
    CHECK(to_string(Priority::VeryHigh) == "very_high");
    CHECK(to_string(Priority::Low) == "low");
    CHECK(from_string<Priority>("very_high") == Priority::VeryHigh);
}

#endif

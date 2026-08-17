// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// ers
#include <erslib/core/enum.hpp>
#include <erslib/core/convert/string.hpp>


using namespace ers::enum_utils;


namespace {
    enum class Color {
        Red [[=as_is]],
        DarkGreen [[=custom<"forest">]],
        LightBlue,
    };

    enum class [[=set_cases<pascal_case_t, upper_snake_case_t>]] Status {
        NotStarted,
        InProgress,
        Done,
    };

    enum class [[=target_case<kebab_case_t>]] Direction {
        NorthWest,
        SouthEast,
    };

    enum class [[=target_case<camel_case_t>]] Weekday {
        MondayStart,
        FridayEnd,
    };
}


TEST_CASE("enum reflection: default case (pascal -> pascal)") {
    CHECK(to_string(Color::Red) == "Red");
    CHECK(to_string(Color::DarkGreen) == "forest");
    CHECK(to_string(Color::LightBlue) == "LightBlue");

    CHECK(from_string<Color>("Red") == Color::Red);
    CHECK(from_string<Color>("forest") == Color::DarkGreen);
    CHECK(from_string<Color>("LightBlue") == Color::LightBlue);
    CHECK_FALSE(from_string<Color>("nope").has_value());
}

TEST_CASE("enum reflection: set_cases annotation (pascal -> upper_snake)") {
    CHECK(to_string(Status::NotStarted) == "NOT_STARTED");
    CHECK(to_string(Status::InProgress) == "IN_PROGRESS");
    CHECK(to_string(Status::Done) == "DONE");

    CHECK(from_string<Status>("IN_PROGRESS") == Status::InProgress);
    CHECK_FALSE(from_string<Status>("InProgress").has_value());
}

TEST_CASE("enum reflection: kebab and camel case styles") {
    CHECK(to_string(Direction::NorthWest) == "north-west");
    CHECK(to_string(Direction::SouthEast) == "south-east");
    CHECK(from_string<Direction>("north-west") == Direction::NorthWest);

    CHECK(to_string(Weekday::MondayStart) == "mondayStart");
    CHECK(to_string(Weekday::FridayEnd) == "fridayEnd");
    CHECK(from_string<Weekday>("fridayEnd") == Weekday::FridayEnd);
}

TEST_CASE("enum reflection: ers::convert integration") {
    CHECK(ers::convert::to_str(Status::Done) == "DONE");
    CHECK(ers::convert::to_sv(Color::Red) == "Red");

    {
        auto r = ers::convert::from_str<Status>("NOT_STARTED");
        REQUIRE(r);
        CHECK(*r == Status::NotStarted);
    }

    {
        auto bad = ers::convert::from_str<Status>("bogus");
        CHECK_FALSE(bad);
    }
}

#endif

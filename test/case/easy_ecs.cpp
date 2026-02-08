// std
#include <iostream>

// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/type/general.hpp>
#include <erslib/util/math.hpp>

// ecs
#include <easy_ecs/engine.hpp>

struct Position {
    f64 x, y;

    Position(f64 x, f64 y) :
        x(x),
        y(y) {
    }

    constexpr bool operator==(const Position& other) const {
        return ers::util::equals(x, other.x)
            && ers::util::equals(y, other.y);
    }
};

TEST_CASE("engine test", "[easy_ecs]") {
    ecs::Engine engine;

    SECTION("position testing") {
        auto src = Position(1.0, 2.0);
        auto& ref = engine.add("player", src).as<Position>();

        REQUIRE(src == ref);
        REQUIRE(&src != &ref);
    }
}

// std
#include <iostream>
#include <print>

// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/type/general.hpp>
#include <erslib/util/math.hpp>

// ecs
#include <easy_ecs/registry.hpp>


struct Position {
    f64 x = 0.0, y = 0.0;

    constexpr bool operator==(const Position& other) const {
        return ers::util::equals(x, other.x)
            && ers::util::equals(y, other.y);
    }
};

struct Velocity {
    f64 x = 0.0, y = 0.0;

    constexpr bool operator==(const Position& other) const {
        return ers::util::equals(x, other.x)
            && ers::util::equals(y, other.y);
    }
};


TEST_CASE("registry test", "[easy_ecs]") {
    ecs::Registry r;

    SECTION("adding position") {
        size_t entity_id = r.add_entity("player");
        r.add_component<Position>(entity_id);
    }

    SECTION("group instancing") {
        r.add_group<Position, Velocity>();

        {
            size_t entity_id = r.add_entity("player");
            r.add_component<Position>(entity_id);
            r.add_component<Velocity>(entity_id);
        }

        {
            size_t entity_id = r.add_entity("enemy");
            r.add_component<Position>(entity_id, 2.0, 0.0);
            r.add_component<Velocity>(entity_id);
        }


        /*for (auto [p, v] : r.view_group<Position, Velocity>()) {
            std::println("p {}:{}\tv {}:{}",
                p.x, p.y, v.x, v.y);
        }

        for (auto [id, p, v] : r.view_group_with_entity_id<Position, Velocity>()) {
            std::println("id {}\tp {}:{}\tv {}:{}",
                id, p.x, p.y, v.x, v.y);
        }*/
    }
}

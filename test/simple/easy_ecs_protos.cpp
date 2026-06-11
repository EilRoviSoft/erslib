#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// doctest
#include <doctest/doctest.h>

// easy_ecs
#include "easy_ecs/registry.hpp"

// aengine
#include "aengine/core/proto.hpp"


// Components

namespace {
    struct HealthData {
        int max = 100;
        int hp = 100;
    };
    
    struct SpeedData {
        float value = 1.f;
    };

    struct ArmorData {
        float factor = 0.f;
    };
}

EASY_ECS_COMPONENT(game, Health, HealthData);
EASY_ECS_COMPONENT(game, Speed, SpeedData);
EASY_ECS_COMPONENT(game, Armor, ArmorData);


// Functions

namespace {
    void register_layouts(aengine::PrototypeRegistry& reg) {
        reg.add_layout("unit")

            .component<game::Health>([](const sol::table& t) -> std::optional<HealthData> {
                auto v = t.get<sol::optional<int>>("max_health");
                if (!v) return std::nullopt;
                return HealthData { .max = *v, .hp = *v };
            }, true)
            .component<game::Speed>([](const sol::table& t) -> std::optional<SpeedData> {
                auto v = t.get<sol::optional<float>>("speed");
                if (!v) return std::nullopt;
                return SpeedData { .value = *v };
            }, true)
            .component<game::Armor>([](const sol::table& t) -> std::optional<ArmorData> {
                auto v = t.get<sol::optional<float>>("armor");
                if (!v) return std::nullopt;
                return ArmorData { .factor = *v };
            }, false);
    }
}

namespace {
    constexpr std::string_view k_script = R"(
data:extend({
    {
        type       = "unit",
        name       = "soldier",
        max_health = 100,
        speed      = 1.0,
        armor      = 0.1,
    },
    {
        type   = "unit",
        name   = "fast-soldier",
        parent = "soldier",
        speed  = 2.5,
    },
    {
        type       = "unit",
        name       = "tank",
        max_health = 800,
        speed      = 0.4,
        armor      = 0.8,
    },
})
    )";
}

TEST_CASE("protos_instantiate") {
    sol::state lua;
    
    lua.open_libraries(sol::lib::base);


    aengine::PrototypeRegistry protos;
    ecs::Registry world;

    register_layouts(protos);
    protos.bind_lua(lua);
    lua.script(k_script);


    world.add_group<game::Health, game::Speed>();
    world.add_group<game::Health, game::Speed, game::Armor>();


    SUBCASE("soldier has correct components") {
        size_t eid = protos.instantiate(world, "soldier");

        auto view = world.view_group<game::Health, game::Speed>();
        bool found = false;

        for (auto& [health, speed] : view) {
            found = true;
            CHECK(health.max == 100);
            CHECK(health.hp == 100);
            CHECK(speed.value == doctest::Approx(1.0f));
        }

        CHECK(found);
    }

    SUBCASE("fast-soldier inherits health and armor, overrides speed") {
        protos.instantiate(world, "fast-soldier", "fast-1");
        protos.instantiate(world, "fast-soldier", "fast-2");

        auto view = world.view_group_with_entity_id<game::Health, game::Speed>();
        int count = 0;

        for (auto& [id, health, speed] : view) {
            count++;
            CHECK(health.max == 100);
            CHECK(speed.value == doctest::Approx(2.5f));
        }

        CHECK(count == 2);
    }

    SUBCASE("tank has correct components") {
        size_t eid = protos.instantiate(world, "tank");

        auto view = world.view_group<game::Health, game::Speed, game::Armor>();
        bool found = false;

        for (auto& [health, speed, armor] : view) {
            found = true;
            CHECK(health.max == 800);
            CHECK(speed.value == doctest::Approx(0.4f));
            CHECK(armor.factor == doctest::Approx(0.8f));
        }

        CHECK(found);
    }

    SUBCASE("unknown prototype throws") {
        CHECK_THROWS(protos.instantiate(world, "does-not-exist"));
    }

    SUBCASE("missing required component throws during load") {
        aengine::PrototypeRegistry bad_protos;
        register_layouts(bad_protos);
        bad_protos.bind_lua(lua);

        CHECK_THROWS(lua.script(R"(
data:extend({
    {
        type = "unit",
        name = "broken",
    }
})
        )"));
    }
}

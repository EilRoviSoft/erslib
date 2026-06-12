// ReSharper disable CppClangTidyPerformanceForRangeCopy

// sol
#include <sol/sol.hpp>

// doctest
#include <doctest/doctest.h>

// easy_ecs
#include <easy_ecs/registry.hpp>

// aengine
#include <aengine/core/proto.hpp>
#include <aengine/script/exception.hpp>


// Components

namespace {
    struct HealthData {
        int max = 100;
        int hp = 100;
    };
}

EASY_ECS_COMPONENT(game, Type, std::string);
EASY_ECS_COMPONENT(game, Name, std::string);
EASY_ECS_COMPONENT(game, Health, HealthData);
EASY_ECS_COMPONENT(game, Speed, float);
EASY_ECS_COMPONENT(game, Armor, float);


// Functions

namespace {
    std::exception_ptr global_ex;

    int my_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
        if (maybe_exception) {
            global_ex = std::make_exception_ptr(*maybe_exception);
        } else {
            std::string message(description.data(), description.size());
            global_ex = std::make_exception_ptr(aengine::lua_error(message));
        }

        return sol::stack::push(L, description);
    }

    void register_layouts(aengine::PrototypeRegistry& reg) {
        reg.add_layout("unit")
            .component<game::Health>([](const sol::table& t) -> std::optional<HealthData> {
                auto v = t.get<sol::optional<int>>("max_health");
                if (!v)
                    return std::nullopt;
                return HealthData { .max = *v, .hp = *v };
            }, true)
            .component<game::Speed>([](const sol::table& t) -> std::optional<float> {
                auto v = t.get<sol::optional<float>>("speed");
                if (!v)
                    return std::nullopt;
                return *v;
            }, true)
            .component<game::Armor>([](const sol::table& t) -> std::optional<float> {
                auto v = t.get<sol::optional<float>>("armor");
                if (!v)
                    return std::nullopt;
                return *v;
            }, false);
    }
}

namespace {
    constexpr std::string_view k_script = R"(
data:extend({
    {
        type = "unit",
        name = "soldier",
        max_health = 100,
        speed = 1.0,
        armor = 0.1,
    },
    {
        type = "unit",
        name = "fast-soldier",
        max_health = 75,
        speed = 2.5,
    },
    {
        type = "unit",
        name = "tank",
        max_health = 800,
        speed = 0.4,
        armor = 0.8,
    },
})
    )";
}

TEST_CASE("protos_instantiate") {
    sol::state lua;

    lua.open_libraries(sol::lib::base);
    lua.set_exception_handler(&my_exception_handler);


    aengine::PrototypeRegistry protos;
    ecs::Registry world;

    register_layouts(protos);
    protos.bind_lua(lua);

    for (const auto& definer : ecs::util::component_definers())
        definer(lua);
    ecs::util::component_definers().clear();


    auto pfr = lua.safe_script(k_script, sol::script_pass_on_error);

    if (!pfr.valid()) {
        sol::error e = pfr;
        MESSAGE(std::string(e.what()));
    }


    world.add_group<game::Health, game::Speed>();
    world.add_group<game::Health, game::Speed, game::Armor>();


    SUBCASE("soldier has correct components") {
        std::ignore = protos.instantiate(world, "soldier");

        auto view = world.view_group<game::Health, game::Speed>();
        bool found = false;

        for (auto [health, speed] : view) {
            found = true;
            CHECK(health.max == 100);
            CHECK(health.hp == 100);
            CHECK(speed == doctest::Approx(1.0f));
        }

        CHECK(found);
    }

    SUBCASE("tank has correct components") {
        std::ignore = protos.instantiate(world, "tank");

        auto view = world.view_group<game::Health, game::Speed, game::Armor>();
        bool found = false;

        for (auto [health, speed, armor] : view) {
            found = true;
            CHECK(health.max == 800);
            CHECK(speed == doctest::Approx(0.4f));
            CHECK(armor == doctest::Approx(0.8f));
        }

        CHECK(found);
    }

    SUBCASE("unknown prototype throws") {
        CHECK_THROWS(protos.instantiate(world, "does-not-exist"));
    }
}

// std
#include <memory>
#include <vector>

// ecs
#include <erslib/easy_ecs/registry.hpp>


namespace {
    constexpr float dt = 1.f / 60.f;
}


namespace {
    struct vec3 {
        float x, y, z;
    };


    struct Position {
        using value_type = vec3;
    };
    struct Velocity {
        using value_type = vec3;
    };
    struct Health {
        using value_type = int;
    };
}


namespace {
    class Unit : public ecs::IEntity {
    public:
        vec3 position = { .x = 0, .y = 0, .z = 0 };
        vec3 velocity = { .x = 0, .y = 0, .z = 0 };
        int health = 100;

        explicit Unit(std::string name) : IEntity(std::move(name)) {}


    protected:
        void track_components(ecs::Registry& registry) override {
            registry.track_component(id(), ecs::component_id<Position>(), &position);
            registry.track_component(id(), ecs::component_id<Velocity>(), &velocity);
            registry.track_component(id(), ecs::component_id<Health>(), &health);
        }
    };
}


int main() {
    ecs::Registry registry;

    registry.add_group<Position, Velocity>();
    registry.add_group<Position, Velocity, Health>();


    std::vector<std::unique_ptr<Unit>> units;

    {
        auto& knight_1 = units.emplace_back(std::make_unique<Unit>("knight_1"));
        knight_1->init(registry);

        auto& knight_2 = units.emplace_back(std::make_unique<Unit>("knight_2"));
        knight_2->init(registry);
    }


    units[0]->health -= 10;


    for (auto&& [pos, vel] : registry.view_group<Position, Velocity>()) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
        pos.z += vel.z * dt;
    }


    return 0;
}

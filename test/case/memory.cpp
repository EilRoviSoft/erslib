// std
#include <array>
#include <cstddef>
#include <memory>

// catch2
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/memory/pmr/holder.hpp>
#include <erslib/type/general.hpp>


struct Position {
    f64 x, y;

    Position(f64 x, f64 y) :
        x(x),
        y(y) {
    }
};


TEST_CASE("testing custom allocator", "[memory]") {
    std::array<std::byte, 1024> buffer;
    std::pmr::monotonic_buffer_resource pool(buffer.data(), sizeof(buffer));

    SECTION("general access") {
        auto n = ers::pmr::make_holder<int>(&pool, 42);
        REQUIRE(n);
        REQUIRE(*n == 42);
    }

    SECTION("ecs") {
        auto position = ers::pmr::make_holder<Position>(&pool, 1.0, 2.0);
        REQUIRE(position);
    }
}

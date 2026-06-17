// doctest
#include <doctest/doctest.h>

// std
#include <format>

// sol
#include <sol/sol.hpp>

// aescript
#include <aescript/layout_verifier.hpp>


static constexpr std::string_view script = R"(
item1 = {
    type = "item",
    name = "fish",
    stack_size = 200,
    icon = "fish.png",
    icon_size = 64,
}
item2 = {
    type = "item",
    name = "fish-koi",
    stack_size = 200,
    icons = {
        icon = "fish-koi.png",
        icon_size = 64,
        tint = { b = 128 },
        shift = { x = 16, y = 16 },
        scale = 2.0,
    }
}
)";


namespace {
    using SpriteSizeType = uint16_t;

    class Vector {
    public:
        static auto get_layout() {
            using namespace aescript::properties;

            static aescript::Layout layout = {
                { "x", optional() | with_type<double>() },
                { "y", optional() | with_type<double>() },
            };

            return layout;
        }
    };

    class Color {
    public:
        static auto get_layout() {
            using namespace aescript::properties;

            static aescript::Layout layout = {
                { "r", optional() | with_type<float>() },
                { "g", optional() | with_type<float>() },
                { "b", optional() | with_type<float>() },
                { "a", optional() | with_type<float>() },
            };

            return layout;
        }
    };

    class IconData {
    public:
        static auto get_layout() {
            using namespace aescript::properties;

            static aescript::Layout layout = {
                { "icon", required() | with_type<std::string>() },
                { "icon_size", optional() | with_type<SpriteSizeType>() },
                { "tint", optional() | with_type<Color>() },
                { "shift", optional() | with_type<Vector>() },
                { "scale", optional() | with_type<double>() },
                { "draw_background", optional() | with_type<bool>() },
                { "floating", optional() | with_type<bool>() }
            };

            return layout;
        }
    };


    class ItemPrototype {
    public:
        static auto get_layout() {
            using namespace aescript::properties;

            static aescript::Layout layout = {
                { "type", required() | with_type<std::string>() },
                { "name", required() | with_type<std::string>() },
                { "stack_size", required() | with_type<size_t>() },
                { "icons", optional() | exclusive_with({ "icon", "icon_size" }) | with_type<IconData>() },
                { "icon", optional() | exclusive_with({ "icons" }) | with_type<std::string>() },
                { "icon_size", optional() | exclusive_with({ "icons" }) | with_type<SpriteSizeType>() },
            };

            return layout;
        }
    };
}


TEST_CASE("layout_verifier") {
    sol::state lua;

    auto sfr = lua.safe_script(script);
    if (!sfr.valid()) {
        sol::error e = sfr;
        FAIL(e.what());
    }

    for (std::string_view it : { "item1", "item2" }) {
        auto item = lua.get<std::optional<sol::table>>(it);
        if (!item) {
            FAIL_CHECK(std::format("Table '{}' is not found", it));
            continue;
        }

        auto layout = ItemPrototype::get_layout();
        if (auto r = layout.verify(*item); !r)
            FAIL_CHECK(r.error().to_string());
    }
}

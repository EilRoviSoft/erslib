// doctest
#include <doctest/doctest.h>

// std
#include <array>
#include <format>
#include <limits>
#include <ranges>

// sol
#include <sol/sol.hpp>

// aescript
#include <aescript/layout.hpp>


using namespace aescript;
using namespace properties;


namespace {
    template<std::floating_point T>
    bool equals(T lhs, T rhs) {
        return std::abs(lhs - rhs) < std::numeric_limits<T>::epsilon();
    }
}


namespace {
    using SpriteSizeType = uint16_t;

    struct Vector {
        double x = 0.0, y = 0.0;


        bool operator==(const Vector& other) const {
            return equals(x, other.x) && equals(y, other.y);
        }


        static auto get_layout() {
            static Layout layout = {
                optional_field("x") | with_type<double>() | parse_into(&Vector::x),
                optional_field("y") | with_type<double>() | parse_into(&Vector::y),
            };

            return layout;
        }
    };

    struct Color {
        float r = 0.f, g = 0.f, b = 0.f, a = 0.f;


        bool operator==(const Color& other) const {
            return equals(r, other.r)
                && equals(g, other.g)
                && equals(b, other.b)
                && equals(a, other.a);
        }


        static auto get_layout() {
            static Layout layout = {
                optional_field("r") | with_type<float>() | parse_into(&Color::r),
                optional_field("g") | with_type<float>() | parse_into(&Color::g),
                optional_field("b") | with_type<float>() | parse_into(&Color::b),
                optional_field("a") | with_type<float>() | parse_into(&Color::a),
            };

            return layout;
        }
    };

    struct IconData {
        std::string icon;
        SpriteSizeType icon_size = 64;
        Color tint;
        Vector shift;
        double scale = 1.0;
        bool draw_background = false;
        bool is_floating = false;


        bool operator==(const IconData& other) const {
            return icon == other.icon
                && icon_size == other.icon_size
                && tint == other.tint
                && shift == other.shift
                && equals(scale, other.scale)
                && draw_background == other.draw_background
                && is_floating == other.is_floating;
        }


        static auto get_layout() {
            static Layout layout = {
                required_field("icon") | with_type<std::string>() | parse_into(&IconData::icon),
                optional_field("icon_size") | with_type<SpriteSizeType>() | parse_into(&IconData::icon_size),
                optional_field("tint") | with_type<Color>() | parse_into(&IconData::tint),
                optional_field("shift") | with_type<Vector>() | parse_into(&IconData::shift),
                optional_field("scale") | with_type<double>() | parse_into(&IconData::scale),
                optional_field("draw_background") | with_type<bool>() | parse_into(&IconData::draw_background),
                optional_field("floating") | with_type<bool>() | parse_into(&IconData::is_floating),
            };

            return layout;
        }
    };


    struct ItemPrototype {
        std::string type;
        std::string name;
        size_t stack_size = 0;
        std::vector<IconData> icons;


        bool operator==(const ItemPrototype& other) const {
            return type == other.type
                && name == other.name
                && stack_size == other.stack_size
                && icons == other.icons;
        }


        // TODO: implement parsing for 'icon' and 'icon_size'
        static auto get_layout() {
            static Layout layout = {
                required_field("type")
                | with_type<std::string>()
                | parse_into(&ItemPrototype::type),

                required_field("name")
                | with_type<std::string>()
                | parse_into(&ItemPrototype::name),

                required_field("stack_size")
                | with_type<size_t>()
                | parse_into(&ItemPrototype::stack_size),

                optional_field("icons")
                | exclusive_with({ "icon", "icon_size" })
                | with_type<std::vector<IconData>>()
                | parse_into(&ItemPrototype::icons),

                optional_field("icon")
                | exclusive_with({ "icons" })
                | with_type<std::string>(),

                optional_field("icon_size")
                | exclusive_with({ "icons" })
                | with_type<SpriteSizeType>(),
            };

            return layout;
        }
    };
}


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
    icons = {{
        icon = "fish-koi.png",
        icon_size = 64,
        tint = { b = 128 },
        shift = { x = 16, y = 16 },
        scale = 2.0,
    }},
}
)";

namespace {
    constexpr std::array items_name = { "item1", "item2" };

    const auto& items_expected_value() {
        static std::array value = {
            ItemPrototype {
                .type       = "item",
                .name       = "fish",
                .stack_size = 200,
                .icons      = {}
            },
            ItemPrototype {
                .type       = "item",
                .name       = "fish-koi",
                .stack_size = 200,
                .icons      = {
                    IconData {
                        .icon      = "fish-koi.png",
                        .icon_size = 64,
                        .tint      = Color { .b = 128 },
                        .shift     = { .x = 16, .y = 16 },
                        .scale     = 2.0
                    }
                }
            }
        };

        return value;
    }
}


TEST_CASE("layout_verifier") {
    sol::state lua;


    auto sfr = lua.safe_script(script);
    if (!sfr.valid()) {
        sol::error e = sfr;
        FAIL(e.what());
    }


    for (auto&& [name, expected] : std::views::zip(items_name, items_expected_value())) {
        auto item = lua.get<std::optional<sol::table>>(name);
        if (!item) {
            FAIL_CHECK(std::format("Table '{}' is not found", name));
            continue;
        }

        auto layout = ItemPrototype::get_layout();


        if (auto s = layout.verify(*item); !s) {
            FAIL_CHECK(s.error().to_string());
            continue;
        }


        ItemPrototype proto;

        if (auto s = layout.parse(*item, &proto); !s) {
            FAIL_CHECK(s.error().to_string());
            continue;
        }


        if (proto != expected) {
            FAIL_CHECK(std::format("Extracted prototype '{}' doesn't equal to expected value '{}'",
                proto.name, expected.name));
            continue;
        }
    }
}

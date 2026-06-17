// doctest
#include <doctest/doctest.h>

// std
#include <format>

// sol
#include <sol/sol.hpp>

// aescript
#include <aescript/layout.hpp>
#include <aescript/parsers/vector.hpp>


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
            using namespace aescript::parsers;

            static aescript::Layout layout = {
                { "x", optional() | with_type<double>() | parse_into(&Vector::m_x) },
                { "y", optional() | with_type<double>() | parse_into(&Vector::m_y) },
            };

            return layout;
        }


    protected:
        double m_x = 0.0, m_y = 0.0;
    };

    class Color {
    public:
        static auto get_layout() {
            using namespace aescript::properties;
            using namespace aescript::parsers;

            static aescript::Layout layout = {
                { "r", optional() | with_type<float>() | parse_into(&Color::m_r) },
                { "g", optional() | with_type<float>() | parse_into(&Color::m_g) },
                { "b", optional() | with_type<float>() | parse_into(&Color::m_b) },
                { "a", optional() | with_type<float>() | parse_into(&Color::m_a) },
            };

            return layout;
        }


    protected:
        float m_r = 0.f, m_g = 0.f, m_b = 0.f, m_a = 0.f;
    };

    class IconData {
    public:
        static auto get_layout() {
            using namespace aescript::properties;
            using namespace aescript::parsers;

            static aescript::Layout layout = {
                { "icon", required() | with_type<std::string>() | parse_into(&IconData::m_icon) },
                { "icon_size", optional() | with_type<SpriteSizeType>() | parse_into(&IconData::m_icon_size) },
                { "tint", optional() | with_type<Color>() | parse_into(&IconData::m_tint) },
                { "shift", optional() | with_type<Vector>() | parse_into(&IconData::m_shift) },
                { "scale", optional() | with_type<double>() | parse_into(&IconData::m_scale) },
                { "draw_background", optional() | with_type<bool>() | parse_into(&IconData::m_draw_background) },
                { "floating", optional() | with_type<bool>() | parse_into(&IconData::m_is_floating) }
            };

            return layout;
        }


    protected:
        std::string m_icon;
        SpriteSizeType m_icon_size = 64;
        Color m_tint;
        Vector m_shift;
        double m_scale = 1.0;
        bool m_draw_background = false;
        bool m_is_floating = false;
    };


    class ItemPrototype {
    public:
        // TODO: implement parsing for 'icon' and 'icon_size'
        static auto get_layout() {
            using namespace aescript::properties;
            using namespace aescript::parsers;

            static aescript::Layout layout = {
                { "type", required() | with_type<std::string>() | parse_into(&ItemPrototype::m_type) },
                { "name", required() | with_type<std::string>() | parse_into(&ItemPrototype::m_name) },
                { "stack_size", required() | with_type<size_t>() | parse_into(&ItemPrototype::m_stack_size) },
                { "icons", optional() | exclusive_with({ "icon", "icon_size" }) | with_type<IconData>() | parse_into(&ItemPrototype::m_icons) },
                { "icon", optional() | exclusive_with({ "icons" }) | with_type<std::string>() },
                { "icon_size", optional() | exclusive_with({ "icons" }) | with_type<SpriteSizeType>() },
            };

            return layout;
        }


    protected:
        std::string m_type;
        std::string m_name;
        size_t m_stack_size = 0;
        std::vector<IconData> m_icons;
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


        if (auto s = layout.verify(*item); !s) {
            FAIL_CHECK(s.error().to_string());
            continue;
        }


        ItemPrototype proto;

        if (auto s = layout.parse(*item, &proto); !s) {
            FAIL_CHECK(s.error().to_string());
            continue;
        }
    }
}

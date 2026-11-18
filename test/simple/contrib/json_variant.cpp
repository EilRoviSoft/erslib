// catch2
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

// std
#include <format>
#include <string>
#include <variant>
#include <vector>

// ers
#include <erslib/contrib/json.hpp>
#include <erslib/core/formatter.hpp>
#include <erslib/core/formatter/auto_format.hpp>

// test
#include "support/test_case.hpp"


#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct [[=utl::json_object]] point_t : ers::AutoFormat {
        i64 x = 0;
        i64 y = 0;
    };

    struct [[=utl::json_object]] face_t : ers::AutoFormat {
        std::string family;
        u16 weight = 400;
        ers::maybe<std::string> subset;
    };

    struct [[=utl::json_object, =utl::json_tag("image")]] image_t : ers::AutoFormat {
        ers::maybe<point_t> center;
        ers::maybe<std::string> origin;
    };

    struct [[=utl::json_object, =utl::json_tag("video")]] video_t : ers::AutoFormat {
        ers::maybe<point_t> center;
        u32 poster_id = 0;
    };

    struct [[=utl::json_object, =utl::json_tag("font")]] font_t : ers::AutoFormat {
        std::vector<face_t> faces;
    };

    using meta_t = utl::json_variant<"kind", image_t, video_t, font_t>;

    struct [[=utl::json_object]] untagged_t {};
    struct [[=utl::json_tag("plain")]] not_object_t {};
    struct [[=utl::json_object, =utl::json_tag("image")]] image_twin_t {};
    struct [[=utl::json_object, =utl::json_tag("clash")]] clash_t { std::string kind; };
    struct [[=utl::json_object, =utl::json_tag("skipped")]] skipped_t { [[=utl::json_skip]] std::string kind; };
    struct [[=utl::json_object, =utl::json_tag("renamed")]] renamed_t { [[=utl::json_key("type")]] std::string kind; };

    struct [[=utl::json_object, =utl::json_tag("boom")]] boom_t {
        boom_t() {
            throw 1;
        }
    };

    using fragile_t = utl::json_variant<"kind", image_t, boom_t>;

    struct [[=utl::json_object]] holder_t {
        meta_t meta;
        ers::maybe<meta_t> extra;
        std::vector<meta_t> list;
    };

    template<ers::fixed_string Tag, typename... Ts>
    concept valid_variant = requires { typename utl::json_variant<Tag, Ts...>; };


    meta_t parse(std::string_view text) {
        return utl::read<meta_t>(utl::from_string(text));
    }

    utl::Json dump(const meta_t& what) {
        return utl::write(what);
    }

    meta_t video_of(u32 poster) {
        video_t video;
        video.poster_id = poster;
        return video;
    }

    meta_t font_of(std::string family, u16 weight) {
        font_t font;
        face_t face;

        face.family = std::move(family);
        face.weight = weight;
        font.faces.push_back(std::move(face));
        return font;
    }
}


ERS_TEST_CASE("json_variant: alternatives are checked", "[contrib][compile_time]") {
    static_assert(valid_variant<"kind", image_t, video_t, font_t>);
    static_assert(valid_variant<"kind", image_t>);

    static_assert(!valid_variant<"kind">);
    static_assert(!valid_variant<"kind", image_t, untagged_t>);
    static_assert(!valid_variant<"kind", image_t, not_object_t>);
    static_assert(!valid_variant<"kind", image_t, image_twin_t>);
    static_assert(!valid_variant<"kind", image_t, image_t>);
    static_assert(!valid_variant<"kind", image_t, clash_t>);
    static_assert(valid_variant<"kind", image_t, skipped_t>);
    static_assert(valid_variant<"kind", image_t, renamed_t>);
    static_assert(valid_variant<"type", image_t, video_t>);

    static_assert(utl::JsonVariant<meta_t>);
    static_assert(!utl::JsonVariant<std::variant<image_t, video_t>>);
    static_assert(utl::JsonConvertible<meta_t>);
    static_assert(meta_t::size == 3);
}

ERS_TEST_CASE("json_variant: works as a std::variant", "[contrib]") {
    const meta_t meta = video_of(7);

    CHECK(std::holds_alternative<video_t>(meta));
    CHECK_FALSE(std::holds_alternative<image_t>(meta));
    REQUIRE(std::get_if<video_t>(&meta) != nullptr);
    CHECK(std::get<video_t>(meta).poster_id == 7u);
    CHECK(meta.index() == 1);

    meta_t other;
    CHECK(std::holds_alternative<image_t>(other));

    other = font_of("Lora", 400);
    CHECK(std::holds_alternative<font_t>(other));

    const auto is_video = std::visit([]<typename Alt>(const Alt&) { return std::is_same_v<Alt, video_t>; },
        static_cast<const meta_t::variant_type&>(meta));

    CHECK(is_video);
}


ERS_TEST_CASE("json_variant: write adds the tag", "[contrib]") {
    const auto json = dump(video_of(7));

    CHECK(json["kind"].as_string() == "video");
    CHECK(json["poster_id"].as_integral() == 7);
    CHECK_FALSE(json.contains("center"));

    CHECK(dump(meta_t{})["kind"].as_string() == "image");
    CHECK(dump(meta_t{}).as_object().size() == 1);
}

ERS_TEST_CASE("json_variant: write keeps arrays and nested objects", "[contrib]") {
    const auto json = dump(font_of("Lora", 700));

    CHECK(json["kind"].as_string() == "font");
    REQUIRE(json["faces"].as_array().size() == 1);
    CHECK(json["faces"][0]["family"].as_string() == "Lora");
    CHECK(json["faces"][0]["weight"].as_integral() == 700);
    CHECK_FALSE(json["faces"][0].contains("subset"));
}

ERS_TEST_CASE("json_variant: a valueless variant is not written", "[contrib]") {
    fragile_t fragile;

    try {
        fragile.emplace<boom_t>();
    } catch (int) {
    }

    REQUIRE(fragile.valueless_by_exception());

    CHECK_FALSE(utl::try_write(fragile));
    CHECK_FALSE(utl::maybe_write(fragile));
    CHECK_THROWS_AS(utl::write(fragile), ers::conversion_error);
}


ERS_TEST_CASE("json_variant: round trip", "[contrib]") {
    const auto back = parse(R"({"kind":"video","poster_id":7,"center":{"x":1,"y":2}})");

    REQUIRE(std::holds_alternative<video_t>(back));
    CHECK(std::get<video_t>(back).poster_id == 7u);
    CHECK(std::get<video_t>(back).center->x == 1);

    const auto font = parse(dump(font_of("Lora", 700)).to_string(utl::EFormat::Minimized));

    REQUIRE(std::holds_alternative<font_t>(font));
    CHECK(std::get<font_t>(font).faces.at(0).family == "Lora");
    CHECK(std::get<font_t>(font).faces.at(0).weight == 700);
}

ERS_TEST_CASE("json_variant: the tag decides, not the keys", "[contrib]") {
    const auto image = parse(R"({"kind":"image","poster_id":7})");

    CHECK(std::holds_alternative<image_t>(image));
}

ERS_TEST_CASE("json_variant: unknown keys are ignored, empty maybe is fine", "[contrib]") {
    const auto image = parse(R"({"kind":"image","unknown":1,"origin":null})");

    REQUIRE(std::holds_alternative<image_t>(image));
    CHECK_FALSE(std::get<image_t>(image).origin);
}

ERS_TEST_CASE("json_variant: read errors", "[contrib]") {
    using Catch::Matchers::ContainsSubstring;

    CHECK_THROWS_WITH(parse(R"({})"), ContainsSubstring("no field 'kind'"));
    CHECK_THROWS_WITH(parse(R"({"kind":7})"), ContainsSubstring("expected string"));
    CHECK_THROWS_WITH(parse(R"({"kind":"audio"})"), ContainsSubstring("audio"));
    CHECK_THROWS_WITH(parse(R"({"kind":"audio"})"), ContainsSubstring("image, video, font"));
    CHECK_THROWS_WITH(parse(R"([])"), ContainsSubstring("expected object"));
    CHECK_THROWS_WITH(parse(R"({"kind":"video"})"), ContainsSubstring("poster_id"));
    CHECK_THROWS_WITH(parse(R"({"kind":"video","poster_id":"x"})"), ContainsSubstring("poster_id"));
    CHECK_THROWS_AS(parse(R"({"kind":"video","poster_id":-1})"), ers::conversion_error);
    CHECK_THROWS_WITH(parse(R"({"kind":"font","faces":[{"weight":400}]})"), ContainsSubstring("family"));
}

ERS_TEST_CASE("json_variant: the tag is case sensitive", "[contrib]") {
    CHECK_THROWS_AS(parse(R"({"kind":"Image"})"), ers::conversion_error);
}

ERS_TEST_CASE("json_variant: policies", "[contrib]") {
    const auto good = utl::from_string(R"({"kind":"video","poster_id":3})");
    const auto bad = utl::from_string(R"({"kind":"nope"})");

    CHECK(utl::try_read<meta_t>(good));
    CHECK_FALSE(utl::try_read<meta_t>(bad));
    CHECK(utl::maybe_read<meta_t>(good));
    CHECK_FALSE(utl::maybe_read<meta_t>(bad));
}

ERS_TEST_CASE("json_variant: as a field and in containers", "[contrib]") {
    const auto holder = utl::read<holder_t>(utl::from_string(
        R"({"meta":{"kind":"image"},"extra":null,"list":[{"kind":"video","poster_id":1},{"kind":"image"}]})"));

    CHECK(std::holds_alternative<image_t>(holder.meta));
    CHECK_FALSE(holder.extra);
    CHECK(holder.list.size() == 2);
    CHECK(std::holds_alternative<video_t>(holder.list[0]));

    const auto json = utl::write(holder);

    CHECK(json["list"][0]["kind"].as_string() == "video");
    CHECK(json["list"][1]["kind"].as_string() == "image");
    CHECK_FALSE(json.contains("extra"));
}

ERS_TEST_CASE("json_variant: format", "[contrib]") {
    using Catch::Matchers::ContainsSubstring;

    CHECK_THAT(std::format("{}", video_of(7)), ContainsSubstring("poster_id = 7"));
}

#endif

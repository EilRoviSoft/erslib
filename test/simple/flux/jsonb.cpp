// catch2
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

// std
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/contrib/json.hpp>
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>
#include <erslib/flux/traits/jsonb.hpp>

// test
#include "support/test_case.hpp"


#ifdef ERSLIB_HAS_REFLECTION

using namespace flux::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    struct [[=utl::json_object]] focus_t {
        i64 x = 50;
        i64 y = 50;
    };

    struct [[=utl::json_object, =flux::jsonb]] meta_t {
        ers::maybe<focus_t> focus;
        ers::maybe<u32> poster_id;
        ers::maybe<std::string> origin;
        ers::maybe<std::vector<std::string>> tags;
    };

    struct [[=utl::json_object]] json_only_t {
        i64 a = 0;
    };

    struct [[=flux::jsonb]] jsonb_only_t {
        i64 a = 0;
    };

    struct Doc {
        std::uint32_t id = flux::undefined_id;
        meta_t metadata;
        ers::maybe<meta_t> extra;
    };

    // Stands for pqxx::field: only what the trait asks for.
    struct fake_field {
        ers::maybe<std::string_view> text;

        bool is_null() const {
            return !text;
        }

        template<typename V>
            requires std::same_as<V, std::string_view>
        V as() const {
            return *text;
        }
    };


    bool contains(std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) != std::string_view::npos;
    }

    std::string sql_of(const flux::QueryBuilder& q) {
        auto r = q.to_sql();
        return r ? *r : std::string("<error>");
    }

    meta_t filled() {
        meta_t meta;
        meta.focus = { .x = 12, .y = 80 };
        meta.poster_id = 7;
        meta.origin = "a.png"s;
        meta.tags = { "x", "y" };
        return meta;
    }
}


template<>
struct flux::reflect::Declaration<Doc> : Table<"docs"> {
    Pk<"id"> docs_pk;
    Default<"metadata", ers::fixed_string("{}")> docs_metadata_default;
};


// Column ----------------------------------------------------------------------

ERS_TEST_CASE("jsonb column", "[flux][compile_time]") {
    using flux::impl::sql_type;

    static_assert(flux::JsonbColumn<meta_t>);
    static_assert(!flux::JsonbColumn<json_only_t>);
    static_assert(!flux::JsonbColumn<jsonb_only_t>);
    static_assert(!flux::JsonbColumn<std::string>);
    static_assert(!flux::JsonbColumn<int>);
    static_assert(!flux::JsonbColumn<ers::maybe<meta_t>>);

    static_assert(sql_type<meta_t>::name == "JSONB");
    static_assert(!sql_type<meta_t>::nullable);

    static_assert(sql_type<ers::maybe<meta_t>>::name == "JSONB");
    static_assert(sql_type<ers::maybe<meta_t>>::nullable);

    static_assert(sql_type<std::string>::name == "TEXT");
}

ERS_TEST_CASE("jsonb create_table", "[flux]") {
    constexpr std::string_view docs = flux::ddl::create_table<Doc>();

    CHECK(contains(docs, "metadata JSONB NOT NULL DEFAULT '{}'"));
    CHECK(contains(docs, "extra JSONB"));
    CHECK_FALSE(contains(docs, "extra JSONB NOT NULL"));
}


// Writing ---------------------------------------------------------------------

ERS_TEST_CASE("jsonb to_text", "[flux]") {
    using flux::impl::sql_value;

    const auto json = utl::from_string(sql_value<meta_t>::to_text(filled()));

    CHECK(json["focus"]["x"].as_integral() == 12);
    CHECK(json["focus"]["y"].as_integral() == 80);
    CHECK(json["poster_id"].as_integral() == 7);
    CHECK(json["origin"].as_string() == "a.png");
    CHECK(json["tags"].as_array().size() == 2);
}

ERS_TEST_CASE("jsonb to_text omits empty optionals", "[flux]") {
    using flux::impl::sql_value;

    const auto json = utl::from_string(sql_value<meta_t>::to_text(meta_t{}));

    CHECK(json.is_object());
    CHECK_FALSE(json.contains("focus"));
    CHECK_FALSE(json.contains("poster_id"));
    CHECK_FALSE(json.contains("origin"));
    CHECK_FALSE(json.contains("tags"));
}

ERS_TEST_CASE("jsonb bind", "[flux]") {
    using flux::impl::sql_value;

    pqxx::params params;
    sql_value<meta_t>::bind(params, filled());
    CHECK(params.size() == 1);

    pqxx::params empty;
    sql_value<ers::maybe<meta_t>>::bind(empty, ers::none);
    CHECK(empty.size() == 1);
}

ERS_TEST_CASE("jsonb in queries", "[flux]") {
    CHECK(sql_of(insert_into("docs") | set("metadata", meta_t{}, "n", 1))
        == "INSERT INTO docs (metadata, n)\nVALUES ($1, $2)");

    CHECK(sql_of(update("docs") | assign("metadata", meta_t{}) | where(col("id") == 1))
        == "UPDATE docs\nSET metadata = $1\nWHERE id = $2");
}


// Reading ---------------------------------------------------------------------

ERS_TEST_CASE("jsonb read", "[flux]") {
    using flux::impl::sql_value;

    const auto full = sql_value<meta_t>::read(fake_field{
        R"({"focus":{"x":1,"y":2},"poster_id":7,"origin":"a.png","tags":["a","b"]})"
        });

    CHECK(full.focus->x == 1);
    CHECK(full.focus->y == 2);
    CHECK(full.poster_id == 7u);
    CHECK(full.origin == "a.png");
    CHECK(full.tags == std::vector<std::string> { "a", "b" });
}

ERS_TEST_CASE("jsonb read defaults", "[flux]") {
    using flux::impl::sql_value;

    const auto bare = sql_value<meta_t>::read(fake_field{ "{}" });

    CHECK_FALSE(bare.focus);
    CHECK_FALSE(bare.poster_id);
    CHECK_FALSE(bare.origin);
    CHECK_FALSE(bare.tags);

    const auto partial = sql_value<meta_t>::read(fake_field { R"({"focus":{"x":9,"y":3}})" });

    REQUIRE(partial.focus);
    CHECK(partial.focus->x == 9);
    CHECK(partial.focus->y == 3);
}

ERS_TEST_CASE("jsonb read tolerates", "[flux]") {
    using flux::impl::sql_value;

    const auto extra = sql_value<meta_t>::read(fake_field{ R"({"unknown":1,"poster_id":3})" });
    CHECK(extra.poster_id == 3u);

    const auto null = sql_value<meta_t>::read(fake_field{ R"({"poster_id":null})" });
    CHECK_FALSE(null.poster_id);
}

ERS_TEST_CASE("jsonb round trip", "[flux]") {
    using flux::impl::sql_value;

    const auto meta = filled();
    const auto text = sql_value<meta_t>::to_text(meta);
    const auto back = sql_value<meta_t>::read(fake_field{ text });

    CHECK(back.focus->x == meta.focus->x);
    CHECK(back.focus->y == meta.focus->y);
    CHECK(back.poster_id == meta.poster_id);
    CHECK(back.origin == meta.origin);
    CHECK(back.tags == meta.tags);
}

ERS_TEST_CASE("jsonb nullable column", "[flux]") {
    using flux::impl::sql_value;

    CHECK_FALSE(sql_value<ers::maybe<meta_t>>::read(fake_field{ ers::none }));

    const auto value = sql_value<ers::maybe<meta_t>>::read(fake_field{ R"({"poster_id":4})" });

    REQUIRE(value);
    CHECK(value->poster_id == 4u);
}

ERS_TEST_CASE("jsonb read errors", "[flux]") {
    using flux::impl::sql_value;

    const auto read_text = [](std::string_view text) {
        static_cast<void>(sql_value<meta_t>::read(fake_field{ text }));
        };

    CHECK_THROWS_AS(read_text("{"), ers::parse_error);

    CHECK_THROWS_AS(read_text("[]"), ers::conversion_error);
    CHECK_THROWS_AS(read_text(R"({"poster_id":"x"})"), ers::conversion_error);
    CHECK_THROWS_AS(read_text(R"({"poster_id":-1})"), ers::conversion_error);
    CHECK_THROWS_AS(read_text(R"({"tags":[1]})"), ers::conversion_error);

    CHECK_THROWS_AS(read_text(R"({"focus":{"x":9}})"), ers::conversion_error);
    CHECK_THROWS_WITH(read_text(R"({"focus":{"x":9}})"), Catch::Matchers::ContainsSubstring("'y'"));

    CHECK_THROWS_WITH(read_text(R"({"focus":{"x":"a"}})"), Catch::Matchers::ContainsSubstring("focus"));
}

#endif

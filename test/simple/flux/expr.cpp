// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    std::string where_of(const flux::Expr& condition) {
        auto r = (select_from("t") | where(condition)).to_sql();
        return r ? *r : std::string("<error>");
    }

    std::string sql_of(const flux::QueryBuilder& q) {
        auto r = q.to_sql();
        return r ? *r : std::string("<error>");
    }

    constexpr flux::ext::binary_op_t<"%"> trgm_similar {};
    constexpr flux::ext::fn_t<"lower"> lower {};
    constexpr flux::ext::fn_t<"date_trunc"> date_trunc {};
}


// Rendering -------------------------------------------------------------------

ERS_TEST_CASE("comparisons", "[flux]") {
    CHECK(where_of(col("a") == 1) == "SELECT *\nFROM t\nWHERE a = $1");
    CHECK(where_of(col("a") != "x") == "SELECT *\nFROM t\nWHERE a <> $1");
    CHECK(where_of(5 <= col("a")) == "SELECT *\nFROM t\nWHERE $1 <= a");
    CHECK(where_of(col("a") == col("b")) == "SELECT *\nFROM t\nWHERE a = b");
    CHECK(where_of(col("t", "a") > 1) == "SELECT *\nFROM t\nWHERE t.a > $1");
}

ERS_TEST_CASE("and/or", "[flux]") {
    CHECK(where_of(col("a") == 1 && col("b") == 2 && col("c") == 3)
        == "SELECT *\nFROM t\nWHERE a = $1 AND b = $2 AND c = $3");

    // where() sits between ANDs, so a top-level OR is wrapped
    CHECK(where_of(col("a") == 1 || (col("b") == 2 && col("c") == 3))
        == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2 AND c = $3)");

    CHECK(where_of((col("a") == 1 || col("b") == 2) && col("c") == 3)
        == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2) AND c = $3");
}

ERS_TEST_CASE("or in where", "[flux]") {
    const auto sql = (select_from("t")
        | where(col("a") == 1 || col("b") == 2)
        | where(col("c") == 3)).to_sql();

    REQUIRE(sql);
    CHECK(*sql == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2) AND c = $3");
}

ERS_TEST_CASE("arithmetic", "[flux]") {
    CHECK(where_of(col("a") - (col("b") - col("c")) > 0)
        == "SELECT *\nFROM t\nWHERE a - (b - c) > $1");

    CHECK(where_of(col("a") - col("b") - col("c") > 0)
        == "SELECT *\nFROM t\nWHERE a - b - c > $1");

    CHECK(where_of((col("a") + col("b")) * col("c") > 0)
        == "SELECT *\nFROM t\nWHERE (a + b) * c > $1");

    CHECK(where_of(col("a") + col("b") * col("c") > 0)
        == "SELECT *\nFROM t\nWHERE a + b * c > $1");
}

ERS_TEST_CASE("not and null", "[flux]") {
    CHECK(where_of(!(col("a") == 1)) == "SELECT *\nFROM t\nWHERE NOT a = $1");
    CHECK(where_of(!(col("a") == 1 && col("b") == 2)) == "SELECT *\nFROM t\nWHERE NOT (a = $1 AND b = $2)");
    CHECK(where_of(col("a").is_null()) == "SELECT *\nFROM t\nWHERE a IS NULL");
    CHECK(where_of(col("a").is_not_null()) == "SELECT *\nFROM t\nWHERE a IS NOT NULL");
    CHECK(where_of(col("a").is_distinct_from(col("b"))) == "SELECT *\nFROM t\nWHERE a IS DISTINCT FROM b");
    CHECK(where_of((col("a") == 1).is_null()) == "SELECT *\nFROM t\nWHERE a = $1 IS NULL");
}

ERS_TEST_CASE("in and between", "[flux]") {
    CHECK(where_of(col("a").in({ 1, 2, 3 })) == "SELECT *\nFROM t\nWHERE a IN ($1, $2, $3)");
    CHECK(where_of(col("a").in(std::vector<std::string> { "x", "y" })) == "SELECT *\nFROM t\nWHERE a IN ($1, $2)");
    CHECK(where_of(col("a").in(std::vector<int> {})) == "SELECT *\nFROM t\nWHERE FALSE");
    CHECK(where_of(!col("a").in(std::vector<int> {})) == "SELECT *\nFROM t\nWHERE NOT FALSE");
    CHECK(where_of(col("a").between(1, 5)) == "SELECT *\nFROM t\nWHERE a BETWEEN $1 AND $2");
    CHECK(where_of(col("a").like("x%")) == "SELECT *\nFROM t\nWHERE a LIKE $1");
    CHECK(where_of(col("a").ilike("x%")) == "SELECT *\nFROM t\nWHERE a ILIKE $1");
    CHECK(where_of(col("a").any(std::vector<int> { 1, 2 })) == "SELECT *\nFROM t\nWHERE a = ANY($1)");
    CHECK(where_of(col("a").contains(col("b"))) == "SELECT *\nFROM t\nWHERE a @> b");
}

ERS_TEST_CASE("cast and row", "[flux]") {
    CHECK(where_of(col("a").cast("uuid") == col("b")) == "SELECT *\nFROM t\nWHERE a::uuid = b");
    CHECK(where_of((col("a") + col("b")).cast("int") > 1) == "SELECT *\nFROM t\nWHERE (a + b)::int > $1");
    CHECK(where_of(row(col("a"), col("b")) < row(1, 2)) == "SELECT *\nFROM t\nWHERE (a, b) < ($1, $2)");
    CHECK(where_of(col("a") == null()) == "SELECT *\nFROM t\nWHERE a = NULL");
    CHECK_FALSE((select_from("t") | where(col("a").cast("int; --") == 1)).to_sql().has_value());
}

ERS_TEST_CASE("subqueries", "[flux]") {
    const auto sql = (select_from("users")
        | where(col("active") == true)
        | where(col("id").in(select_from("orders") | columns("user_id") | where(col("total") > 100)))
        | where(exists(select_from("bans") | where(col("user_id") == col("users", "id")))
            == false)).to_sql();

    REQUIRE(sql);
    CHECK(sql->contains("WHERE active = $1 AND id IN (SELECT user_id"));
    CHECK(sql->contains("WHERE total > $2"));
    CHECK(sql->contains("EXISTS (SELECT *"));
    CHECK(sql->contains("= $3"));
}

ERS_TEST_CASE("identifiers", "[flux]") {
    CHECK(where_of(col("a; DROP TABLE t") == 1) == "<error>");
    CHECK(where_of(col("a") == "'; DROP TABLE t; --"s) == "SELECT *\nFROM t\nWHERE a = $1");
}


// Extension -------------------------------------------------------------------

ERS_TEST_CASE("user operators", "[flux]") {
    CHECK(where_of(trgm_similar(col("name"), "bob")) == "SELECT *\nFROM t\nWHERE name % $1");
    CHECK(where_of(lower(col("email")) == "a@b.c") == "SELECT *\nFROM t\nWHERE lower(email) = $1");
    CHECK(where_of(date_trunc("day", col("at")) == col("d")) == "SELECT *\nFROM t\nWHERE date_trunc($1, at) = d");
    CHECK(where_of(trgm_similar(col("a"), col("b")) && col("c") == 1) == "SELECT *\nFROM t\nWHERE a % b AND c = $1");
}

ERS_TEST_CASE("raw sql", "[flux]") {
    CHECK(where_of(sql("{} @@ plainto_tsquery('russian', {})", col("search"), "q"))
        == "SELECT *\nFROM t\nWHERE (search @@ plainto_tsquery('russian', $1))");

    CHECK(where_of(sql("a OR b") && col("c") == 1) == "SELECT *\nFROM t\nWHERE (a OR b) AND c = $1");
    CHECK(where_of(sql("{}")) == "<error>");
    CHECK(where_of(sql("x", 1)) == "<error>");
}

ERS_TEST_CASE("custom node", "[flux]") {
    struct ZeroLess final : flux::ext::IExprNode {
        flux::Expr value;

        explicit ZeroLess(flux::Expr v) : value(std::move(v)) {}

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.query += "0 < ";
            return value.render(ctx, flux::ext::EPrecedence::Comparison);
        }

        flux::ext::EPrecedence precedence() const noexcept override { return flux::ext::EPrecedence::Comparison; }
    };

    const auto custom = flux::ext::make_expr<ZeroLess>(col("x") + col("y"));

    CHECK(where_of(custom && col("z") == 1) == "SELECT *\nFROM t\nWHERE 0 < x + y AND z = $1");
}

ERS_TEST_CASE("custom clause", "[flux]") {
    class DistinctOnClause final : public flux::ext::IClause {
    public:
        explicit DistinctOnClause(std::vector<flux::Expr> on) :
            IClause(&flux::slots::distinct),
            _on(std::move(on)) {
        }

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.query += " DISTINCT ON (";

            for (size_t i = 0; i < _on.size(); ++i) {
                if (i > 0)
                    ctx.query += ", ";

                ERS_TRY(_on[i].render(ctx));
            }

            ctx.query += ')';
            return ers::ok;
        }

    private:
        std::vector<flux::Expr> _on;
    };

    CHECK(sql_of(select_from("prices")
        | flux::ext::make_clause<DistinctOnClause>(std::vector { col("product_id") })
        | order_by("product_id")
        | order_by("at", EOrder::Desc))
        == "SELECT DISTINCT ON (product_id) *\nFROM prices\nORDER BY product_id ASC, at DESC");
}


// Parameters ------------------------------------------------------------------

ERS_TEST_CASE("param", "[flux]") {
    const auto at = param(5);

    CHECK(where_of(col("a") > at && col("b") < at && col("c") == 5)
        == "SELECT *\nFROM t\nWHERE a > $1 AND b < $1 AND c = $2");
}

ERS_TEST_CASE("arrays", "[flux]") {
    CHECK(where_of(any_or_empty(col("id"), std::vector<std::int64_t> { 1, 2 }))
        == "SELECT *\nFROM t\nWHERE (cardinality($1::BIGINT[]) = 0 OR id = ANY($1))");

    CHECK(where_of(empty_array(std::vector<std::string> {}))
        == "SELECT *\nFROM t\nWHERE cardinality($1::TEXT[]) = 0");
}


// Statements ------------------------------------------------------------------

ERS_TEST_CASE("select list", "[flux]") {
    CHECK(sql_of(select_from("orders")
        | columns("user_id", count_all().as("n"), sum_of("total").as("sum"))
        | column(coalesce(max_of("total"), 0).as("top"))
        | group_by("user_id")
        | having(count_all() > 5)
        | order_by(sum_of("total"), EOrder::Desc))
        == "SELECT user_id, count(*) AS n, sum(total) AS sum, coalesce(max(total), $1) AS top"
           "\nFROM orders"
           "\nGROUP BY user_id"
           "\nHAVING count(*) > $2"
           "\nORDER BY sum(total) DESC");
}

ERS_TEST_CASE("aggregates", "[flux]") {
    CHECK(sql_of(select_from("t") | columns(count_distinct("a"), corr_of("y", "x"), agg<"regr_slope">("y", col("x") * 2)))
        == "SELECT count(DISTINCT a), corr(y, x), regr_slope(y, x * $1)\nFROM t");

    CHECK(sql_of(select_from("t") | column(string_agg("name", ", ")))
        == "SELECT string_agg(name, $1)\nFROM t");

    CHECK(sql_of(select_from("t") | column(percentile_cont(0.5, { "price" })))
        == "SELECT percentile_cont($1) WITHIN GROUP (ORDER BY price)\nFROM t");

    CHECK(sql_of(select_from("t") | column(mode_of({ "a" })))
        == "SELECT mode() WITHIN GROUP (ORDER BY a)\nFROM t");

    CHECK(sql_of(select_from("items") | column(json_rows("items", { "position" })))
        == "SELECT json_agg(row_to_json(items) ORDER BY position)\nFROM items");

    CHECK(sql_of(select_from("t") | column(mode_of({}))) == "<error>");
}

ERS_TEST_CASE("select subquery", "[flux]") {
    const auto orders = select_from("orders") | column(count_all()) | where(col("user_id") == col("u.id"));

    CHECK(sql_of(select_from("users", "u") | columns("u.id", subquery(orders).as("orders")))
        == "SELECT u.id, (SELECT count(*)\nFROM orders\nWHERE user_id = u.id) AS orders\nFROM users AS u");
}

ERS_TEST_CASE("order by", "[flux]") {
    CHECK(sql_of(select_from("t") | order_by(col("a") + col("b")) | order_by("c", EOrder::Desc))
        == "SELECT *\nFROM t\nORDER BY a + b ASC, c DESC");

    CHECK(sql_of(select_from("t") | order_by_stable_random("id", 42))
        == "SELECT *\nFROM t\nORDER BY md5(id::text || $1::text) ASC");

    CHECK(sql_of(select_from("t") | order_by_random()) == "SELECT *\nFROM t\nORDER BY random() ASC");
}

ERS_TEST_CASE("assign", "[flux]") {
    CHECK(sql_of(update("products") | assign("stock", col("stock") - 3) | assign("seen_at", now()) | where(col("id") == 1))
        == "UPDATE products\nSET stock = stock - $1, seen_at = now()\nWHERE id = $2");

    CHECK(sql_of(update("t") | assign_null("a")) == "UPDATE t\nSET a = NULL");

    CHECK(sql_of(insert_into("t") | columns("a", "at") | values("x", now()) | returning("id", col("at").as("created")))
        == "INSERT INTO t (a, at)\nVALUES ($1, now())\nRETURNING id, at AS created");
}

ERS_TEST_CASE("bad alias", "[flux]") {
    CHECK(sql_of(select_from("t") | column(col("a").as("x; --"))) == "<error>");
}

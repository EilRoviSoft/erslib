// doctest
#include <doctest/doctest.h>

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>


using namespace flux::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    std::string where_of(const flux::Expr& condition) {
        auto r = (select_from("t") | where(condition)).to_sql();
        return r ? *r : std::string("<error>");
    }

    constexpr flux::ext::binary_op_t<"%"> trgm_similar {};
    constexpr flux::ext::fn_t<"lower"> lower {};
    constexpr flux::ext::fn_t<"date_trunc"> date_trunc {};
}


// Rendering -------------------------------------------------------------------

TEST_CASE("flux expr: comparisons bind values") {
    CHECK(where_of(col("a") == 1) == "SELECT *\nFROM t\nWHERE a = $1");
    CHECK(where_of(col("a") != "x") == "SELECT *\nFROM t\nWHERE a <> $1");
    CHECK(where_of(5 <= col("a")) == "SELECT *\nFROM t\nWHERE $1 <= a");
    CHECK(where_of(col("a") == col("b")) == "SELECT *\nFROM t\nWHERE a = b");
    CHECK(where_of(col("t", "a") > 1) == "SELECT *\nFROM t\nWHERE t.a > $1");
}

TEST_CASE("flux expr: and/or get parentheses only where precedence needs them") {
    CHECK(where_of(col("a") == 1 && col("b") == 2 && col("c") == 3)
        == "SELECT *\nFROM t\nWHERE a = $1 AND b = $2 AND c = $3");

    // where() sits between ANDs, so a top-level OR is wrapped
    CHECK(where_of(col("a") == 1 || (col("b") == 2 && col("c") == 3))
        == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2 AND c = $3)");

    CHECK(where_of((col("a") == 1 || col("b") == 2) && col("c") == 3)
        == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2) AND c = $3");
}

TEST_CASE("flux expr: an or is wrapped next to other where clauses") {
    const auto sql = (select_from("t")
        | where(col("a") == 1 || col("b") == 2)
        | where(col("c") == 3)).to_sql();

    REQUIRE(sql);
    CHECK(*sql == "SELECT *\nFROM t\nWHERE (a = $1 OR b = $2) AND c = $3");
}

TEST_CASE("flux expr: arithmetic keeps its grouping") {
    CHECK(where_of(col("a") - (col("b") - col("c")) > 0)
        == "SELECT *\nFROM t\nWHERE a - (b - c) > $1");

    CHECK(where_of(col("a") - col("b") - col("c") > 0)
        == "SELECT *\nFROM t\nWHERE a - b - c > $1");

    CHECK(where_of((col("a") + col("b")) * col("c") > 0)
        == "SELECT *\nFROM t\nWHERE (a + b) * c > $1");

    CHECK(where_of(col("a") + col("b") * col("c") > 0)
        == "SELECT *\nFROM t\nWHERE a + b * c > $1");
}

TEST_CASE("flux expr: not, null checks and distinct") {
    CHECK(where_of(!(col("a") == 1)) == "SELECT *\nFROM t\nWHERE NOT a = $1");
    CHECK(where_of(!(col("a") == 1 && col("b") == 2)) == "SELECT *\nFROM t\nWHERE NOT (a = $1 AND b = $2)");
    CHECK(where_of(col("a").is_null()) == "SELECT *\nFROM t\nWHERE a IS NULL");
    CHECK(where_of(col("a").is_not_null()) == "SELECT *\nFROM t\nWHERE a IS NOT NULL");
    CHECK(where_of(col("a").is_distinct_from(col("b"))) == "SELECT *\nFROM t\nWHERE a IS DISTINCT FROM b");
    CHECK(where_of((col("a") == 1).is_null()) == "SELECT *\nFROM t\nWHERE a = $1 IS NULL");
}

TEST_CASE("flux expr: in, between, like, any") {
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

TEST_CASE("flux expr: cast, row and null") {
    CHECK(where_of(col("a").cast("uuid") == col("b")) == "SELECT *\nFROM t\nWHERE a::uuid = b");
    CHECK(where_of((col("a") + col("b")).cast("int") > 1) == "SELECT *\nFROM t\nWHERE (a + b)::int > $1");
    CHECK(where_of(row(col("a"), col("b")) < row(1, 2)) == "SELECT *\nFROM t\nWHERE (a, b) < ($1, $2)");
    CHECK(where_of(col("a") == null()) == "SELECT *\nFROM t\nWHERE a = NULL");
    CHECK_FALSE((select_from("t") | where(col("a").cast("int; --") == 1)).to_sql().has_value());
}

TEST_CASE("flux expr: subqueries share the placeholder counter") {
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

TEST_CASE("flux expr: identifiers are validated, values are bound") {
    CHECK(where_of(col("a; DROP TABLE t") == 1) == "<error>");
    CHECK(where_of(col("a") == "'; DROP TABLE t; --"s) == "SELECT *\nFROM t\nWHERE a = $1");
}


// Extension -------------------------------------------------------------------

TEST_CASE("flux expr: operators and functions are added by the user") {
    CHECK(where_of(trgm_similar(col("name"), "bob")) == "SELECT *\nFROM t\nWHERE name % $1");
    CHECK(where_of(lower(col("email")) == "a@b.c") == "SELECT *\nFROM t\nWHERE lower(email) = $1");
    CHECK(where_of(date_trunc("day", col("at")) == col("d")) == "SELECT *\nFROM t\nWHERE date_trunc($1, at) = d");
    CHECK(where_of(trgm_similar(col("a"), col("b")) && col("c") == 1) == "SELECT *\nFROM t\nWHERE a % b AND c = $1");
}

TEST_CASE("flux expr: raw sql is always parenthesized and takes arguments") {
    CHECK(where_of(sql("{} @@ plainto_tsquery('russian', {})", col("search"), "q"))
        == "SELECT *\nFROM t\nWHERE (search @@ plainto_tsquery('russian', $1))");

    CHECK(where_of(sql("a OR b") && col("c") == 1) == "SELECT *\nFROM t\nWHERE (a OR b) AND c = $1");
    CHECK(where_of(sql("{}")) == "<error>");
    CHECK(where_of(sql("x", 1)) == "<error>");
}

TEST_CASE("flux expr: a custom node plugs in") {
    struct ZeroLess final : flux::ext::IExprNode {
        flux::Expr value;

        explicit ZeroLess(flux::Expr v) : value(std::move(v)) {}

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.query += "0 < ";
            return value.render(ctx, flux::ext::EPrecedence::Comparison);
        }

        flux::ext::EPrecedence precedence() const noexcept override { return flux::ext::EPrecedence::Comparison; }
    };

    const flux::Expr custom(std::make_shared<const ZeroLess>(col("x") + col("y")));

    CHECK(where_of(custom && col("z") == 1) == "SELECT *\nFROM t\nWHERE 0 < x + y AND z = $1");
}


// Old and new clauses ---------------------------------------------------------

TEST_CASE("flux expr: coexists with the old where clauses") {
    const auto sql = (select_from("t")
        | where("a", "="_op, 1)
        | where(col("b") == 2 || col("c") == 3)).to_sql();

    REQUIRE(sql);
    CHECK(*sql == "SELECT *\nFROM t\nWHERE a = $1 AND (b = $2 OR c = $3)");
}

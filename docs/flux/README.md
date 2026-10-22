# flux

PostgreSQL on top of libpqxx: a connection pool, a query builder and conditions as expressions. Tables from structs are in [reflect](../reflect/README.md).

```cpp
#include <erslib/flux.hpp>

using namespace flux::orm;      // layouts, clauses, col(), ...
```


## Connecting

```cpp
flux::db_options_t opts {
    .host     = "localhost",
    .port     = 5432,
    .dbname   = "shop",
    .username = "shop",
    .password = "...",
};

flux::Database db(opts.connection_string(), {
    .min_size        = 1,
    .max_size        = 4,
    .acquire_timeout = std::chrono::seconds(5),
    .idle_timeout    = std::chrono::minutes(5),
});

db.maintain();      // drops idle connections older than idle_timeout, keeps min_size; call it from a timer
```


## Transactions

```cpp
// committed when the lambda returns, rolled back when it throws
auto users = flux::with_tx(db, [](pqxx::read_transaction& tx) {
    return (select_from("users") | with_limit(10)).exec(tx).get_as<std::vector<User>>();
}, "Users/SelectFirst");    // the name shows up in errors

// the transaction type comes from the lambda's parameter, or is given explicitly
flux::with_tx<pqxx::work>(conn, [](auto& tx) { ... });

// one statement, its own transaction
flux::with_query(db, delete_from("sessions") | where(col("expires_at") < col("now")), "Sessions/Purge");

// a lambda returning a failed ers::Result rolls back too
flux::with_tx(db, [&](pqxx::work& tx) -> ers::Status {
    ERS_TRY(reserve(tx, item));
    return charge(tx, item);
});
```

`try_*` never throw, a pqxx exception becomes `ers::Result`:

```cpp
auto count = flux::try_with_tx(db, [](pqxx::work& tx) {
    return (select_from("users") | named(count_all(), "n")).exec(tx).get_as<i64>();
}, "Users/Count");                                    // ers::Result<i64>

auto users = flux::try_with_query(db, select_from("users"), "Users/All")
    .get_as<std::vector<User>>();                     // ers::Result<std::vector<User>>

auto done = flux::try_invoke([&] { ... }, "Label");   // any pqxx call
```

```cpp
flux::with_conn(db, [](pqxx::connection& conn) { ... });     // just the connection, no transaction
auto conn = db.acquire_or_throw();                          // back to the pool when it goes out of scope
```

Anything with `acquire_or_throw()` returning a movable handle with `*handle -> pqxx::connection&` works as `db` here (`flux::ConnectionSource`).


## Statements

```cpp
select_from("users", "u")
    | columns("u.id", "u.name")
    | join("orders", "o", col("o.user_id") == col("u.id"))
    | where(col("u.active") == true && col("o.total") > 100)
    | group_by("u.id")
    | order_by("u.name")                // EOrder::Desc
    | with_limit(20)
    | with_offset(40);
```

```sql
SELECT u.id, u.name
FROM users AS u
JOIN orders AS o ON o.user_id = u.id
WHERE u.active = $1 AND o.total > $2
GROUP BY u.id
ORDER BY u.name ASC
LIMIT $3
OFFSET $4
```

```cpp
insert_into("images")
    | columns("url", "path")
    | values(img.url, img.path)
    | on_conflict("url")                // or on_conflict("a", "b"), on_conflict_constraint("images_url_key")
    | do_update("path")                 // SET path = excluded.path; or do_nothing()
    | returning("id");                  // returning_all() for RETURNING *

update("users")
    | assign("name", name)              // set() is the same
    | assign_null("email")
    | where(col("id") == id);

delete_from("sessions") | where(col("user_id") == id);

exists_in("users") | where(col("email") == email);     // SELECT EXISTS (...) AS "exists", read as bool
```

Values are always parameters. Identifiers are checked (`a-z A-Z 0-9 _ $ .`), a bad one makes `to_sql()`/`exec()` fail instead of reaching the database.

```cpp
auto q = select_from("users") | where(col("id") == 1);

q.to_sql();         // ers::Result<std::string>, for logs and tests
q.exec(tx);         // flux::QueryResult
q |= with_limit(1); // builders are values, add to them in place or copy them
```


### More clauses

```cpp
select_from("orders")
    | named(count_all(), "n")                               // count(*) AS n
    | named(sum_of("total"), "sum")                         // also min_of, max_of, avg_of, count_of, count_distinct,
    | named(coalesce_with(max_of("total"), sql_bool(false)), "m")  // array_agg_of, bool_and_of, stddev_of, ...
    | group_by("user_id");

select_from("items") | json_rows("items", { "position" });  // json_agg(row_to_json(items) ORDER BY position)
select_from("u") | named(subquery(select_from("o") | count_all()), "orders");

select_from("products") | order_by_stable_random("id", seed);    // same order for the same seed, for paging
select_from("products") | order_by_random();
select_from("users") | distinct();

select_from("recent") | with_cte("recent", select_from("orders") | where(col("created_at") > since));
select_from("a") | columns("id") | union_with(select_from("b") | columns("id"), true);     // UNION ALL

select_from("users")
    | when(only_active, where(col("active") == true))      // skipped when false
    | when(with_bans, [&] { return join("bans", col("bans.user_id") == col("users.id")); });  // built only if true
```


## Conditions

`col()` is an identifier, anything else is a bound value. `==`, `&&` and the rest build SQL, not `bool`.

```cpp
col("a") == 1                           // a = $1
col("a") != col("b")                    // a <> b
5 <= col("a")                           // $1 <= a
col("t", "a")                           // t.a

col("a") == 1 && (col("b") == 2 || col("c") == 3)   // a = $1 AND (b = $2 OR c = $3)
!(col("a") == 1)                        // NOT a = $1
col("a") - (col("b") - col("c")) > 0    // parentheses only where PostgreSQL needs them

col("a").is_null()                      // .is_not_null(), .is_distinct_from(x)
col("a").like("x%")                     // .ilike()
col("a").between(1, 5)
col("a").in({ 1, 2, 3 })                // a IN ($1, $2, $3)
col("a").in(ids)                        // any range; an empty one renders FALSE
col("a").in(select_from("t") | columns("id"))
col("a").any(ids)                       // a = ANY($1), the whole vector is one parameter
col("tags").contains(other)             // @>
col("id").cast("uuid")                  // id::uuid

exists(select_from("bans") | where(col("bans.user_id") == col("users.id")))
row(col("created_at"), col("id")) < row(at, id)     // keyset pagination
null()                                  // NULL; compare with .is_null(), '= NULL' is never true
val(x)                                  // a value where an Expr is expected
```

```cpp
// one value, one placeholder, however many times it's used
auto at = param(cursor);
col("a") > at && col("b") < at                      // a > $1 AND b < $1

any_or_empty(col("category"), ids)                  // (cardinality($1::BIGINT[]) = 0 OR category = ANY($1))
empty_array(ids)                                    // cardinality($1::BIGINT[]) = 0
```

`where()` and `join()` take only an `Expr`: `where("flag")` doesn't compile instead of binding a string. Several `where()` are joined with `AND`, an `OR` inside one of them gets parentheses.

```cpp
std::vector<flux::Expr> filters;

if (query.min_price)
    filters.emplace_back(col("price") >= *query.min_price);

if (query.text)
    filters.emplace_back(col("title").ilike("%" + *query.text + "%"));

auto q = select_from("products");
for (auto& it : filters)
    q |= where(it);
```


## Results

```cpp
result.get_as<User>()                       // exactly one row, throws otherwise
result.get_as<ers::maybe<User>>()           // 0..1
result.get_as<std::vector<User>>()          // 0..N
result.get_as<i64>()                        // one row, one column
result.view_as<User>()                      // lazy, 0..N
result.raw()                                // pqxx::result
```

How a row becomes a struct is in [reflect](../reflect/README.md#reading-rows).


## Extending

An operator or a function: the name is a template argument, so it can't come from user input.

```cpp
inline constexpr flux::ext::binary_op_t<"%"> trgm_similar;                               // pg_trgm
inline constexpr flux::ext::binary_op_t<"<->", flux::ext::EPrecedence::Other> distance;  // pgvector
inline constexpr flux::ext::fn_t<"lower"> lower;
inline constexpr flux::ext::fn_t<"date_trunc"> date_trunc;

where(trgm_similar(col("name"), query))             // name % $1
where(lower(col("email")) == email)                 // lower(email) = $1
where(date_trunc("day", col("at")) == col("d"))     // date_trunc($1, at) = d
```

Everything else, as text. The pattern has to be a literal, `{}` takes the next argument:

```cpp
where(sql("{} @@ plainto_tsquery('russian', {})", col("search"), text))
// (search @@ plainto_tsquery('russian', $1)), always in parentheses
```

A whole node:

```cpp
class CaseNode final : public flux::ext::IExprNode {
public:
    CaseNode(flux::Expr cond, flux::Expr then, flux::Expr otherwise);

    ers::Status render(flux::ext::Context& ctx) const override {
        ctx.query += "CASE WHEN ";
        ERS_TRY(_cond.render(ctx));             // render(ctx, precedence) adds parentheses when needed
        ctx.query += " THEN ";
        ERS_TRY(_then.render(ctx));
        ctx.query += " ELSE ";
        ERS_TRY(_otherwise.render(ctx));
        ctx.query += " END";
        return ers::ok;
    }

    // flux::ext::EPrecedence precedence() const noexcept override;   // Primary by default

private:
    flux::Expr _cond, _then, _otherwise;
};

auto price = flux::ext::make_expr<CaseNode>(col("sale").is_null(), col("price"), col("sale"));
where(price < 1000);
```

`ctx.bind(binder)` returns the next `$N` for a value, `flux::ext::make_binder(value)` makes one.

A clause:

```cpp
class OrderByExprClause final : public flux::ext::IClause {
public:
    explicit OrderByExprClause(flux::Expr what) :
        IClause(&flux::slots::order),           // the slot decides where in the statement it goes
        _what(std::move(what)) {
    }

    ers::Status render(flux::ext::Context& ctx) const override {
        return _what.render(ctx);
    }

private:
    flux::Expr _what;
};

select_from("items")
    | flux::ext::make_clause<OrderByExprClause>(distance(col("embedding"), vec))
    | with_limit(10);
// ORDER BY embedding <-> $1
```

`raw()` is the quick version of the same, `?` marks values, nothing in the text is checked:

```cpp
select_from("t") | raw(&flux::slots::where, "a = ? OR b = ?", 1, 2);     // WHERE a = $1 OR b = $2
```

A value of your own type: `flux::sql_type<T>` and `flux::sql_value<T>`, see [reflect](../reflect/README.md#your-own-types).

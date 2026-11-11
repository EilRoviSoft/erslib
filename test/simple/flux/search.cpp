// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>
#include <erslib/flux/search/pattern.hpp>
#include <erslib/flux/traits/tsvector.hpp>

#ifdef ERSLIB_HAS_REFLECTION
#   include <erslib/flux/reflect.hpp>
#endif

// test
#include "support/test_case.hpp"


using namespace flux::orm;

using flux::search::ELike;


// Details ---------------------------------------------------------------------

namespace {
    std::string where_of(const flux::Expr& condition) {
        auto r = (select_from("t") | where(condition)).to_sql();
        return r ? *r : std::string("<error>");
    }

    flux::Expr shop_query() {
        return to_tsquery(ts_config("shop_ru"), "x");
    }
}


// Basic tests -----------------------------------------------------------------

ERS_TEST_CASE("like pattern", "[flux]") {
    CHECK(flux::search::like_pattern("abc") == "%abc%");
    CHECK(flux::search::like_pattern("") == "%%");

    CHECK(flux::search::like_pattern("a_b%") == R"(%a\_b\%%)");
    CHECK(flux::search::like_pattern("a_b%", ELike::Prefix) == R"(a\_b\%%)");
    CHECK(flux::search::like_pattern("a_b%", ELike::Suffix) == R"(%a\_b\%)");
    CHECK(flux::search::like_pattern("a_b%", ELike::Exact) == R"(a\_b\%)");

    CHECK(flux::search::like_pattern(R"(a\b)") == R"(%a\\b%)");
    CHECK(flux::search::like_pattern("платье") == "%платье%");
}

ERS_TEST_CASE("tsquery word", "[flux]") {
    CHECK(flux::search::tsquery_word("плат") == "'плат':*");
    CHECK(flux::search::tsquery_word("плат", false) == "'плат'");

    CHECK(flux::search::tsquery_word("o'neil") == "'o''neil':*");
    CHECK(flux::search::tsquery_word(R"(a\b)", false) == R"('a\\b')");

    CHECK(flux::search::tsquery_word("x & !y") == "'x & !y':*");
    CHECK(flux::search::tsquery_word("a:*|(b)") == "'a:*|(b)':*");
}

ERS_TEST_CASE("tsquery words", "[flux]") {
    const std::vector<std::string> words = { "плат", "", "красн" };

    CHECK(flux::search::tsquery_words(words) == "'плат':* & 'красн':*");
    CHECK(flux::search::tsquery_words(words, " | ") == "'плат':* | 'красн':*");
    CHECK(flux::search::tsquery_words(words, " & ", false) == "'плат' & 'красн'");

    CHECK(flux::search::tsquery_words(std::vector<std::string> {}).empty());
    CHECK(flux::search::tsquery_words(std::vector<std::string> { "", "" }).empty());

    constexpr std::array<std::string_view, 2> views = { "a", "b'c" };
    CHECK(flux::search::tsquery_words(views) == "'a':* & 'b''c':*");
}

ERS_TEST_CASE("full text expressions", "[flux]") {
    CHECK(where_of(ts_match(col("document"), shop_query()))
        == "SELECT *\nFROM t\nWHERE document @@ to_tsquery($1::regconfig, $2)");

    CHECK(where_of(col("a") == 1 && ts_match(col("document"), shop_query()))
        == "SELECT *\nFROM t\nWHERE a = $1 AND document @@ to_tsquery($2::regconfig, $3)");

    CHECK(where_of(ts_rank_cd(col("document"), shop_query()) > 0)
        == "SELECT *\nFROM t\nWHERE ts_rank_cd(document, to_tsquery($1::regconfig, $2)) > $3");

    CHECK(where_of(setweight(to_tsvector(ts_config("shop_ru"), col("title")), "A") == col("d"))
        == "SELECT *\nFROM t\nWHERE setweight(to_tsvector($1::regconfig, title), $2) = d");
}

ERS_TEST_CASE("trigram expressions", "[flux]") {
    CHECK(where_of(trgm_similar(col("title"), "bob"))
        == "SELECT *\nFROM t\nWHERE title % $1");

    CHECK(where_of(trgm_word_similar(col("w"), col("title")))
        == "SELECT *\nFROM t\nWHERE w <% title");

    CHECK(where_of(similarity(col("title"), "bob") > 0)
        == "SELECT *\nFROM t\nWHERE similarity(title, $1) > $2");

    CHECK(where_of(unaccent(col("title")) == col("d"))
        == "SELECT *\nFROM t\nWHERE unaccent(title) = d");
}


// Reflection ------------------------------------------------------------------

#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct SearchSchema : flux::reflect::Schema<"public"> {
        flux::reflect::Extension<"pg_trgm"> trgm;
        flux::reflect::Extension<"unaccent"> unaccent;
    };

    struct EmptySchema : flux::reflect::Schema<"empty"> {};

    struct Doc {
        u32 item_id;
        std::string title;
        flux::tsvector document;
    };

    struct DefaultedDoc {
        u32 item_id;
        flux::tsvector document;
    };

    struct Plain {
        u32 id;
    };
}

template<>
struct flux::reflect::Declaration<Doc> : Table<"docs", SearchSchema> {
    Pk<"item_id"> docs_pk;
    ReadOnly<"document"> docs_document_read_only;

    Index<EIndexMethod::Gin, "document"> docs_document;
    Index<EIndexMethod::Gin, "title gin_trgm_ops", "title <> ''"> docs_title_trgm;
};

template<>
struct flux::reflect::Declaration<DefaultedDoc> : Table<"defaulted_docs", SearchSchema> {
    Unique<"item_id"> defaulted_docs_item_unique;
    Default<"document", empty_tsvector> defaulted_docs_document_default;
};

template<>
struct flux::reflect::Declaration<Plain> : Table<"plain", SearchSchema> {
    Pk<"id"> plain_pk;
};


using flux::impl::reflect::extension_statements;
using flux::impl::reflect::index_statements;


ERS_TEST_CASE("index method names", "[flux][compile_time]") {
    using flux::reflect::EIndexMethod;

    static_assert(ers::enum_utils::to_string(EIndexMethod::BTree) == "btree");
    static_assert(ers::enum_utils::to_string(EIndexMethod::Hash) == "hash");
    static_assert(ers::enum_utils::to_string(EIndexMethod::Gin) == "gin");
    static_assert(ers::enum_utils::to_string(EIndexMethod::Gist) == "gist");
    static_assert(ers::enum_utils::to_string(EIndexMethod::Brin) == "brin");
}

ERS_TEST_CASE("index statements", "[flux][compile_time]") {
    static_assert(index_statements<Doc>.size() == 2);

    static_assert(index_statements<Doc>[0]
        == "CREATE INDEX IF NOT EXISTS docs_document ON public.docs USING gin (document)");

    static_assert(index_statements<Doc>[1]
        == "CREATE INDEX IF NOT EXISTS docs_title_trgm ON public.docs USING gin (title gin_trgm_ops) WHERE title <> ''");

    static_assert(index_statements<Plain>.empty());
}

ERS_TEST_CASE("extension statements", "[flux][compile_time]") {
    static_assert(extension_statements<SearchSchema>.size() == 2);
    static_assert(extension_statements<SearchSchema>[0] == "CREATE EXTENSION IF NOT EXISTS pg_trgm");
    static_assert(extension_statements<SearchSchema>[1] == "CREATE EXTENSION IF NOT EXISTS unaccent");

    static_assert(extension_statements<EmptySchema>.empty());
}

ERS_TEST_CASE("tsvector column", "[flux]") {
    CHECK(flux::ddl::create_table<Doc>().contains("document TSVECTOR NOT NULL"));
    CHECK(flux::ddl::create_table<DefaultedDoc>().contains("document TSVECTOR NOT NULL DEFAULT ''::tsvector"));
}

#endif

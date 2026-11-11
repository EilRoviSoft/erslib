#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/flux/expr/ops.hpp>


// Macro -----------------------------------------------------------------------

#define ERS_FLUX_SEARCH_EXPRS(X) \
    X(to_tsvector) \
    X(to_tsquery) \
    X(plainto_tsquery) \
    X(websearch_to_tsquery) \
    X(setweight) \
    X(ts_rank) \
    X(ts_rank_cd) \
    X(ts_headline) \
    X(numnode) \
    X(similarity) \
    X(word_similarity) \
    X(unaccent)

#define ERS_FLUX_EXPR_FN(NAME) \
    inline constexpr fn_t<#NAME> NAME {};


namespace flux::impl::clauses {
    ERS_FLUX_SEARCH_EXPRS(ERS_FLUX_EXPR_FN);


    inline constexpr binary_op_t<"@@"> ts_match {};

    inline constexpr binary_op_t<"%"> trgm_similar {};
    inline constexpr binary_op_t<"<%"> trgm_word_similar {};
    inline constexpr binary_op_t<"<->"> trgm_distance {};


    inline Expr ts_config(std::string_view name) {
        return Expr(std::string(name)).cast("regconfig");
    }
}

#undef ERS_FLUX_SEARCH_EXPRS
#undef ERS_FLUX_EXPR_FN

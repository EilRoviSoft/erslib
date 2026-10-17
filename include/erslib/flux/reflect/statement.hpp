#pragma once

// ers
#include <erslib/flux/reflect/statement/query_call.hpp>
#include <erslib/flux/reflect/statement/traits.hpp>


// Macros ----------------------------------------------------------------------

#define ERS_DECLARE_QUERY(NS, NAME, LABEL) \
    namespace NS { \
        struct NAME##_tag {}; \
        inline constexpr auto NAME = flux::reflect::query_fn<NAME##_tag>; \
    } \
    \
    template<> \
    struct flux::reflect::Declaration<NS::NAME##_tag> : flux::reflect::Query<LABEL>

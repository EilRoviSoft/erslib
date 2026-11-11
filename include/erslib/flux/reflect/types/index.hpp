#pragma once

// std
#include <string>

// ers
#include <erslib/core/enum.hpp>
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>
#include <erslib/flux/reflect/types/entry.hpp>


namespace flux::impl::reflect {
    enum class EIndexMethod : u8 {
        BTree,
        Hash,
        Gin,
        Gist,
        Brin,
    };
}

template<>
struct ers::enum_traits<flux::impl::reflect::EIndexMethod> {
    using from = lower_case_t;
    using to = lower_case_t;
};


namespace flux::impl::reflect {
    struct index_context {
        std::string_view name;
        std::string_view table;
    };


    template<EIndexMethod Method, ers::fixed_string Columns, ers::fixed_string Where = "">
    struct Index {
        using kind = kinds::EIndex;
    };

    template<EIndexMethod Method, ers::fixed_string Columns, ers::fixed_string Where>
    struct entry_traits<Index<Method, Columns, Where>> : entry_defaults {
        static consteval std::string index_ddl(const index_context& ctx) {
            std::string out = "CREATE INDEX IF NOT EXISTS " + std::string(ctx.name)
                + " ON " + std::string(ctx.table)
                + " USING " + std::string(ers::enum_utils::to_string(Method))
                + " (" + std::string(Columns.to_sv()) + ')';

            if constexpr (!Where.to_sv().empty())
                out += " WHERE " + std::string(Where.to_sv());

            return out;
        }
    };


    template<typename E>
    concept TableIndex = requires(const index_context& ctx) {
        { entry_traits<E>::index_ddl(ctx) } -> std::convertible_to<std::string>;
    };
}

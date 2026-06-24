#pragma once

// std
#include <filesystem>
#include <string_view>

// ers
#include <erslib/core/thread_safe/map.hpp>

// dbio
#include <erslib/dbio/fwd.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    // Thread-safe registry mapping query labels to their SQL text.
    // Generated code looks queries up by label, e.g. queries["sql.user.save"].
    class ERSLIB_EXPORT QueryStore : public ers::thread_safe::Map<internal::kv_container_type> {
    public:
        // Member functions

        QueryStore() = default;


        // Modifiers

        // "sql.<relative/path/without-ext>" with '/' replaced by '.'.
        size_t load_directory(const std::filesystem::path& root);

        void add(std::string_view label, std::string_view query);
    };
}

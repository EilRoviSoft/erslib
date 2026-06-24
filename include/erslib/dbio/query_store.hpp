#pragma once

// std
#include <filesystem>
#include <string>
#include <unordered_map>

// ers
#include <erslib/core/thread_safe/map.hpp>

// export
#include <erslib/core/export.hpp>


namespace dbio {
    // Thread-safe registry mapping query labels to their SQL text.
    // Generated code looks queries up by label, e.g. queries["sql.user.save"].
    class ERSLIB_EXPORT QueryStore : public ers::thread_safe::Map<std::unordered_map<std::string, std::string>> {
    public:
        QueryStore() = default;

        // Recursively walks a folder of generated.sql files
        // and registers each one under "sql.<relative/path/without-ext>" with '/' replaced by '.',
        // so <root>/user/save.sql  ->  "sql.user.save"
        size_t load_directory(const std::filesystem::path& root);

        // Registers a single label -> sql pair (overwrites if present).
        void add(const std::string& label, std::string sql);
    };
}

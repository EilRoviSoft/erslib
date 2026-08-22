#pragma once

// std
#include <filesystem>

// ers
#include <erslib/core/thread_safe/map.hpp>
#include <erslib/dbio/fwd.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    // Thread-safe registry mapping query labels to their SQL text.
    // Generated code looks queries up by label, e.g. queries["user.save"].
    class ERSLIB_EXPORT QueryStore : public ers::thread_safe::Map<kv_container_type> {
    public:
        // Member functions

        QueryStore() = default;


        static QueryStore make_from_path(const std::filesystem::path& path);


        // Modifiers

        size_t load_directory(const std::filesystem::path& root);

        void add(std::string_view label, std::string_view query);
    };


    extern QueryStore queries;
}


// Exports

namespace dbio {
    using impl::QueryStore;
    using impl::queries;
}

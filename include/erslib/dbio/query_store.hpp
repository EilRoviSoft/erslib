#pragma once

// std
#include <filesystem>

// ers
#include <erslib/core/thread_safe.hpp>
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

        // Recursively loads every "*.sql" file under root
        // as "sql.<relative/path/without-ext>" with '/' replaced by '.'.
        size_t load_directory(const std::filesystem::path& root);

        void add(std::string_view label, std::string_view query);
    };


#ifdef ERS_DBIO_GLOBAL_QUERY_STORE
    // Defined only when ERSLIB_DBIO_OWN_QUERY_STORE is on; it loads ./res/query at static init.
    extern QueryStore queries;
#endif
}


// Exports

namespace dbio {
    using impl::QueryStore;

#ifdef ERS_DBIO_GLOBAL_QUERY_STORE
    using impl::queries;
#endif
}

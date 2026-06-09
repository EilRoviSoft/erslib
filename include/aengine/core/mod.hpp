#pragma once

// std
#include <exception>
#include <filesystem>
#include <string>

// boost
#include <boost/unordered/unordered_set.hpp>

// sol
#include <sol/environment.hpp>
#include <sol/state_view.hpp>

// erslib
#include <erslib/type/version.hpp>

// aengine
#include <aengine/fwd.hpp>
#include <aengine/core/dependency.hpp>


namespace fs = std::filesystem;


// Internal

namespace aengine::internal {
    struct ModIdentity {
        std::string name;
        std::string title;
        ers::version_t version = { 0, 0, 0 };
    };


    struct ModMetadata {
        std::string author;
        std::string contact;
        std::string description;
        DependencyContainer dependencies;
    };


    struct ModContent {
        StringMap<std::string> packages;
        StringMap<std::string> stages;
    };


    struct ModRuntime {
        StringMap<sol::object> modules_cache;
        sol::protected_function main;
        sol::environment env;
        std::exception_ptr pending_exception = nullptr;
    };
}


// Mod

namespace aengine {
    class Mod {
    public:
        // Types definition

        using identity_type = internal::ModIdentity;
        using metadata_type = internal::ModMetadata;
        using content_type = internal::ModContent;
        using runtime_type = internal::ModRuntime;


        // Constructor

        explicit Mod(fs::path dir);


        // Modifiers

        void init_info();

        void drop_metadata() const { m_metadata.reset(); }

        void init_content(const std::function<bool(std::string_view)>& stage_filter) const;
        void drop_content() const { m_content.reset(); }

        void init_runtime(sol::state_view& lua) const;
        void load_runtime(std::string_view stage_name) const;
        void drop_runtime() const { m_runtime.reset(); }


        // Accessors

        std::string_view name() const { return m_identity.name; }
        std::string_view title() const { return m_identity.title; }
        const ers::version_t& version() const { return m_identity.version; }

        const metadata_type& metadata() const { return *m_metadata; }

        const content_type& content() const { return *m_content; }
        
        const runtime_type& runtime() const { return *m_runtime; }


    protected:
        fs::path m_dir;
        identity_type m_identity;
        mutable std::unique_ptr<metadata_type> m_metadata;
        mutable std::unique_ptr<content_type> m_content;
        mutable std::unique_ptr<runtime_type> m_runtime;


    private:
        std::function<sol::object(sol::this_state, std::string_view)> _make_require_fn() const;
    };


    using ModContainer = boost::unordered_set<
        Mod,
        ers::member_string_hash_adaptor<ers::RapidHash, &Mod::name>,
        ers::member_equal_adaptor<&Mod::name>
    >;
}

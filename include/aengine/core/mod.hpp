#pragma once

// std
#include <filesystem>
#include <string>

// erslib
#include <erslib/adaptor/transparent/base.hpp>
#include <erslib/type/result.hpp>
#include <erslib/type/version.hpp>

// aengine
#include <aengine/core/dependency.hpp>
#include <aengine/fwd.hpp>


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


    class ModContent {
        struct phase_order {
            bool operator()(std::string_view lhs, std::string_view rhs) const;
        };
        using phase_comparator = ers::adaptor::binary_op<std::string, phase_order>;


    public:
        const auto& packages() const { return m_packages; }
        const auto& stages() const { return m_stages; }

        void load(const fs::path& dir);


    private:
        StringMap<std::string> m_packages;
        OrderedMap<std::string, std::string, phase_comparator> m_stages;
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


        // Constructor

        explicit Mod(fs::path dir);


        // Modifiers

        void load_info();
        void load_content() const;

        void drop_metadata() const;
        void drop_content() const;


        // Accessors

        std::string_view name() const { return m_identity.name; }
        std::string_view title() const { return m_identity.title; }
        const ers::version_t& version() const { return m_identity.version; }


        const identity_type& identity() const { return m_identity; }
        const metadata_type& metadata() const { return *m_metadata; }
        const content_type& content() const { return *m_content; }


    protected:
        fs::path m_dir;
        identity_type m_identity;
        mutable std::unique_ptr<metadata_type> m_metadata;
        mutable std::unique_ptr<content_type> m_content;
    };


    using ModContainer = HashSet<
        Mod,
        ers::member_hash_adaptor<ers::hashing::rapid_policy, &Mod::name>,
        ers::member_equal_adaptor<&Mod::name>
    >;
}

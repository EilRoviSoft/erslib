#pragma once

// std
#include <filesystem>

// quill
#include <quill/Logger.h>

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/pattern/proxy.hpp>
#include <erslib/type/result.hpp>

// aengine
#include <aengine/core/manager.hpp>
#include <aengine/core/mod.hpp>
#include <aengine/core/proto.hpp>

// ecs
#include <easy_ecs/registry.hpp>


namespace fs = std::filesystem;


// Forward declaration

namespace aengine {
    class Context;
}


// Core

namespace aengine {
    class Core {
        friend class Context;


    public:
        // Constructor

        explicit Core(fs::path cwd = "./");


        // Destructor

        virtual ~Core() = default;


        // Modifiers

        virtual ers::Status init() = 0;


    protected:
        fs::path m_cwd;
        quill::Logger* m_logger = nullptr;
        ModContainer m_mods;
        sol::state m_lua;
        ecs::Registry m_registry;
        ResourceManager m_resources;
        LayoutDictionary m_layouts;
    };
}


// Context

namespace aengine {
    class Context : public ers::pattern::TProxy<Core> {
    public:
        // Constructor

        Context(Core& value);


        // Observers

        const fs::path& cwd() const { return get()->m_cwd; }
        quill::Logger* logger() { return get()->m_logger; }
        ModContainer& mods() { return get()->m_mods; }
        sol::state_view lua() { return static_cast<sol::state_view>(get()->m_lua); }
        ecs::Registry& registry() { return get()->m_registry; }
        ResourceManager& resources() { return get()->m_resources; }
        LayoutDictionary& layouts() { return get()->m_layouts; }
    };
}

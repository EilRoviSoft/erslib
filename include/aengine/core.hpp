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

        virtual ~Core();


        // Modifiers

        virtual void init() = 0;


    protected:
        fs::path m_cwd;
        ecs::Registry m_registry;
        LayoutDictionary m_layouts;
        ResourceManager m_resources;
        ModContainer m_mods;
        sol::state m_lua;
        quill::Logger* m_logger = nullptr;
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

        ecs::Registry& registry() { return get()->m_registry; }

        LayoutDictionary& layouts() { return get()->m_layouts; }

        ResourceManager& resources() { return get()->m_resources; }

        sol::state_view lua() { return static_cast<sol::state_view>(get()->m_lua); }

        ModContainer& mods() { return get()->m_mods; }

        quill::Logger* logger() { return get()->m_logger; }
    };
}

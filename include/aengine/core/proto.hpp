#pragma once

// std
#include <utility>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/adaptor/transparent/base.hpp>

// ecs
#include <easy_ecs/registry.hpp>
#include <easy_ecs/util/component_initializer.hpp>

// aengine
#include <aengine/fwd.hpp>


// Layout

namespace aengine {
    // In the best world you should allocate 'Layout' once and never change it again.
    // Otherwise, this could lead to UB.
    class Layout {
    public:
        // Member types

        using Applier = std::function<void(ecs::Registry&, size_t entity_id)>;

        struct ComponentEntry {
            size_t id;
            std::string name;
            bool required;

            // Returns nullopt when the relevant Lua fields are absent,
            // allowing prototype-level inheritance to supply the value.
            std::function<std::optional<Applier>(const sol::table&)> make_applier;
        };


        // Member functions

        explicit Layout(std::string_view name);


        // Modifiers

        template<ecs::ComponentLike T, typename Deserializer>
        Layout& component(Deserializer&& d, bool required = true) {
            m_entries.push_back({
                .id           = ecs::component_id<T>(),
                .name         = std::string(ecs::component_name<T>()),
                .required     = required,
                .make_applier =
                [d = std::forward<Deserializer>(d)](const sol::table& table) -> std::optional<Applier> {
                    auto r = d(table);
                    if (!r)
                        return std::nullopt;

                    auto shared = std::make_shared<typename T::value_type>(std::move(*r));

                    return Applier([shared](ecs::Registry& reg, size_t id) {
                        reg.add_component<T>(id, *shared);
                    });
                }
            });
            return *this;
        }

        Layout& inherit(const Layout& parent);


        // Accessors

        std::string_view name() const { return m_name; }
        const std::vector<ComponentEntry>& entries() const { return m_entries; }


    protected:
        std::string m_name;
        std::vector<ComponentEntry> m_entries;
    };
}


// Prototype

namespace aengine {
    class Prototype {
    public:
        // Member types

        using Applier = Layout::Applier;


        // Member functions

        Prototype() = default;
        Prototype(std::string_view type, std::string_view name);


        // Modifiers

        void inherit_from(const Prototype& parent);

        void load(const Layout& layout, const sol::table& table);

        size_t instantiate(ecs::Registry& registry) const;

        size_t instantiate(ecs::Registry& registry, std::string_view entity_name) const;


        // Accessors

        std::string_view type() const { return m_type; }
        std::string_view name() const { return m_name; }

        bool has_component(size_t id) const { return m_appliers.contains(id); }


    protected:
        std::string m_type;
        std::string m_name;

        // key = component_id<T>()
        TrivialMap<Applier> m_appliers;
    };
}


// PrototypeRegistry

namespace aengine {
    class PrototypeRegistry {
    public:
        // Modifiers

        Layout& add_layout(std::string_view name);


        // Load one table: must have 'type' and 'name'; 'parent' is optional.
        void load(const sol::table& table);

        // Load an array of prototype tables (data:extend({...}) style).
        void load_all(const sol::table& array);


        size_t instantiate(ecs::Registry& registry, std::string_view proto_name) const;
        size_t instantiate(ecs::Registry& registry, std::string_view proto_name, std::string_view entity_name) const;


        // Injects a global 'data' table with an 'extend' method.
        void bind_lua(sol::state_view& lua);


        // Accessors

        Layout& layout(std::string_view name) { return m_layouts.at(name); }
        const Layout& layout(std::string_view name) const { return m_layouts.at(name); }

        bool has_layout(std::string_view name) const { return m_layouts.contains(name); }


        const Prototype& prototype(std::string_view name) const { return m_prototypes.at(name); }

        bool has_prototype(std::string_view name) const { return m_prototypes.contains(name); }


    private:
        StringMap<Layout> m_layouts;
        StringMap<Prototype> m_prototypes;
    };
}

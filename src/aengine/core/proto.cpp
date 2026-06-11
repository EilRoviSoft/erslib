#include "aengine/core/proto.hpp"

// std
#include <ranges>

// ers
#include <erslib/exception/runtime_error.hpp>

// ecs
#include <easy_ecs/util/component_initializer.hpp>


// Layout

aengine::Layout::Layout(std::string_view name) :
    m_name(name) {
}

aengine::Layout& aengine::Layout::inherit(const Layout& parent) {
    m_entries.insert_range(m_entries.begin(), parent.m_entries);
    return *this;
}


// Prototype

aengine::Prototype::Prototype(std::string_view type, std::string_view name) :
    m_type(type),
    m_name(name) {
}


void aengine::Prototype::inherit_from(const Prototype& parent) {
    for (const auto& [id, applier] : parent.m_appliers)
        m_appliers.try_emplace(id, applier);
}

void aengine::Prototype::load(const Layout& layout, const sol::table& table) {
    for (const auto& entry : layout.entries()) {
        if (auto maybe = entry.make_applier(table)) {
            m_appliers.insert_or_assign(entry.id, std::move(*maybe));
        } else if (!m_appliers.contains(entry.id) && entry.required) {
            throw ers::make_runtime_error("Prototype '{}' (type '{}'): required component '{}' is absent and has no inherited value",
                m_name, m_type, entry.name);
        }
    }
}

size_t aengine::Prototype::instantiate(ecs::Registry& registry) const {
    return instantiate(registry, m_name);
}
size_t aengine::Prototype::instantiate(ecs::Registry& registry, std::string_view entity_name) const {
    size_t id = registry.add_entity(entity_name);
    for (const auto& applier : m_appliers | std::views::values)
        applier(registry, id);
    return id;
}


// PrototypeRegistry

aengine::Layout& aengine::PrototypeRegistry::add_layout(std::string_view name) {
    auto [it, _] = m_layouts.emplace(name, Layout(name));
    return it->second;
}

void aengine::PrototypeRegistry::load(const sol::table& table) {
    auto type = table.get<std::string>("type");
    auto name = table.get<std::string>("name");

    if (!has_layout(type))
        throw ers::make_runtime_error("Unknown prototype type: '{}'", type);
    
    if (has_prototype(name))
        throw ers::make_runtime_error("Prototype '{}' already registered", name);


    // Emplace first; no further layout insertions can happen here,
    // so the layout reference below stays valid.

    auto [it, _] = m_prototypes.emplace(name, Prototype { type, name });
    Prototype& proto = it->second;

    proto.load(layout(type), table);
}
void aengine::PrototypeRegistry::load_all(const sol::table& array) {
    for (const auto& v : array | std::views::values) {
        if (v.is<sol::table>())
            load(v.as<sol::table>());
    }
}

size_t aengine::PrototypeRegistry::instantiate(ecs::Registry& registry, std::string_view proto_name) const {
    return prototype(proto_name).instantiate(registry);
}
size_t aengine::PrototypeRegistry::instantiate(ecs::Registry& registry, std::string_view proto_name, std::string_view entity_name) const {
    return prototype(proto_name).instantiate(registry, entity_name);
}

void aengine::PrototypeRegistry::bind_lua(sol::state_view& lua) {
    sol::table data = lua.create_named_table("data");
    data.set_function("extend", [this](const sol::table&, const sol::table& protos) {
        load_all(protos);
    });
}

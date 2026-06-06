#include "aengine/core/dependency_graph.hpp"

// std
#include <deque>

// erslib
#include <erslib/views.hpp>


void aengine::DependencyGraph::add_initial_mod(std::string_view name) {
    m_initial_mods.emplace_back(name);
}

void aengine::DependencyGraph::resolve_order(const ModContainer& mods) {
    // Step 0: preparing

    StringMap<std::vector<std::string>> outgoing;
    StringMap<size_t> indegree;


    m_execution_order.clear();


    // Step 1: registering every mod

    for (const auto& mod : mods) {
        outgoing.try_emplace(mod.identity().name);
        indegree.emplace(mod.identity().name, 0);
    }


    // Step 2: Building graph

    for (const Mod& mod : mods) {
        for (const dependency_t& dep : mod.metadata().dependencies) {
            auto it = mods.find(dep.name);
            bool dependency_exist = it != mods.end(),
                 add_to_graph = false;


            // It wil be converted to jump table anyway and as a switch-case this block would be much clankier.

            if (dep.type == DependencyType::Required) {
                if (!dependency_exist) {
                    throw dependency_error("Required dependency '{}' for mod '{}' is missing.",
                        dep.name, mod.identity().name);
                }

                add_to_graph = true;
            } else if (dep.type == DependencyType::Optional) {
                if (dependency_exist)
                    continue;

                add_to_graph = true;
            } else if (dep.type == DependencyType::Incompatible) {
                if (dependency_exist) {
                    throw dependency_error("Mod '{}' is incompatible with loaded mod '{}'.",
                        mod.identity().name, dep.name);
                }
            }


            if (add_to_graph) {
                outgoing[dep.name].emplace_back(mod.identity().name);
                indegree[mod.identity().name]++;
            }
        }
    }


    // Step 3: Kahn queue

    std::deque<std::string> ready;
    StringSet queued;

    for (const auto& name : m_initial_mods) {
        auto it = indegree.find(name);

        if (it == indegree.end())
            throw dependency_error("Initial mod '{}' does not exist. Files are probably corrupted.", name);

        if (it->second != 0)
            throw dependency_error("Initial mod '{}' unexpectedly has dependencies.", name);

        ready.emplace_back(name);
        queued.emplace(name);
    }

    for (const auto& [name, degree] : indegree) {
        if (degree != 0 || queued.contains(name))
            continue;

        ready.emplace_back(name);
        queued.emplace(name);
    }


    // Step 4: Topological sort

    size_t visited_count = 0;

    while (!ready.empty()) {
        auto current = std::move(ready.front());
        ready.pop_front();

        m_execution_order.emplace_back(current);
        visited_count++;

        for (const auto& dependent : outgoing.at(current)) {
            auto it = indegree.find(dependent);

            if (it == indegree.end())
                continue;

            it->second--;

            if (it->second != 0 || queued.contains(dependent))
                continue;

            ready.emplace_back(dependent);
            queued.emplace(dependent);
        }
    }


    // Step 5: Finishing with finding dependency cycles

    if (visited_count != indegree.size()) {
        throw dependency_error("Dependency cycle detected between mods: {}",
            indegree
            | ers::views::filter([](const auto& it) { return it.second != 0; })
            | std::views::keys);
    }
}

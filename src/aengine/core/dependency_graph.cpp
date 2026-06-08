#include "aengine/core/dependency_graph.hpp"

// std
#include <deque>
#include <ranges>

// erslib
#include <erslib/views.hpp>


namespace {
    auto parse_phase_and_index(std::string_view sv) {
        aengine::Phase phase = *ers::convert::from_str_constexpr<aengine::Phase>(sv);
        size_t off = ers::convert::from_string_backend<aengine::Phase>::conversion_table[static_cast<size_t>(phase) - 1].first.size() + 1;
        auto r = ers::convert::from_str<size_t>(sv.substr(off));

        if (!r)
            throw ers::conversion_error(r.error().to_string(true));

        return std::make_pair(phase, *r);
    };
}


std::vector<std::string> aengine::resolve_mods_order(const ModContainer& mods, std::string_view initial_mod) {
    // Step 0: preparing

    StringMap<std::vector<std::string>> outgoing;
    StringMap<size_t> indegree;


    // Step 1: registering every mod

    for (const auto& mod : mods) {
        outgoing.try_emplace(mod.name());
        indegree.emplace(mod.name(), 0);
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
                        dep.name, mod.name());
                }

                add_to_graph = true;
            } else if (dep.type == DependencyType::Optional) {
                if (dependency_exist)
                    continue;

                add_to_graph = true;
            } else if (dep.type == DependencyType::Incompatible) {
                if (dependency_exist) {
                    throw dependency_error("Mod '{}' is incompatible with loaded mod '{}'.",
                        mod.name(), dep.name);
                }
            }


            if (add_to_graph) {
                outgoing[dep.name].emplace_back(mod.name());
                indegree[mod.name()]++;
            }
        }
    }


    // Step 3: Kahn queue

    std::deque<std::string> ready;
    StringSet queued;

    {
        auto it = indegree.find(initial_mod);

        if (it == indegree.end())
            throw dependency_error("Initial mod '{}' does not exist. Files are probably corrupted.", initial_mod);

        if (it->second != 0)
            throw dependency_error("Initial mod '{}' unexpectedly has dependencies.", initial_mod);

        ready.emplace_back(initial_mod);
        queued.emplace(initial_mod);
    }

    for (const auto& [name, degree] : indegree) {
        if (degree != 0 || queued.contains(name))
            continue;

        ready.emplace_back(name);
        queued.emplace(name);
    }


    // Step 4: Topological sort

    std::vector<std::string> mods_order;
    size_t visited_count = 0;

    mods_order.reserve(mods.size());

    while (!ready.empty()) {
        auto current = std::move(ready.front());
        ready.pop_front();

        mods_order.emplace_back(current);
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
        throw dependency_error("Occured dependency cycle detected between mods: {}",
            indegree
            | ers::views::filter([](const auto& it) { return it.second != 0; })
            | std::views::keys);
    }

    return mods_order;
}

std::vector<aengine::internal::stage_info_t> aengine::resolve_stages_order(
    const ModContainer& mods,
    std::span<const std::string> mods_order
) {
    static constexpr auto phases = { Phase::Settings, Phase::Data, Phase::Scripts };

    using indexes_per_phase_t = std::array<OrderedSet<size_t>, static_cast<size_t>(Phase::Max) - 1>;
    StringMap<indexes_per_phase_t> indexes_per_mod;
    size_t stages_count = 0;

    for (const auto& mod : mods) {
        for (const auto& stage : mod.content().stages | std::views::keys) {
            auto [phase, index] = parse_phase_and_index(stage);
            indexes_per_mod[mod.name()][static_cast<size_t>(phase) - 1].emplace(index);
            stages_count++;
        }
    }


    std::vector<internal::stage_info_t> stages_order;
    stages_order.reserve(stages_count);

    for (const auto& phase : phases) {
        for (const auto& mod : mods_order) {
            for (const auto& index : indexes_per_mod[mod][static_cast<size_t>(phase) - 1]) {
                stages_order.emplace_back(internal::stage_info_t {
                    .mod   = mod,
                    .phase = phase,
                    .index = index
                });
            }
        }
    }


    return stages_order;
}

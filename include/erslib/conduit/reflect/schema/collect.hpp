#pragma once

// std
#include <meta>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// ers
#include <erslib/conduit/reflect/declaration.hpp>
#include <erslib/conduit/reflect/schema/entity.hpp>
#include <erslib/conduit/reflect/schema/members.hpp>
#include <erslib/conduit/reflect/types.hpp>


// Discovery

namespace conduit::impl::reflect {
    consteval void collect_row_types(std::meta::info ns, std::vector<std::meta::info>& out) {
        const auto members = std::meta::members_of(ns, std::meta::access_context::unchecked());

        for (const auto it : members) {
            if (std::meta::is_namespace_alias(it))
                continue;

            if (std::meta::is_namespace(it)) {
                collect_row_types(it, out);
                continue;
            }

            if (!std::meta::is_type(it) || std::meta::is_type_alias(it) || !std::meta::is_class_type(it))
                continue;

            if (std::meta::is_complete_type(std::meta::substitute(^^Declaration, { it })))
                out.emplace_back(it);
        }
    }

    consteval std::span<const std::meta::info> row_types_in(std::meta::info ns) {
        std::vector<std::meta::info> out;
        collect_row_types(ns, out);

        return std::define_static_array(out);
    }
}


// Planning

namespace conduit::impl::reflect {
    struct schema_plan_t {
        std::span<const std::meta::info> entities;
        std::string_view error;
    };


    template<typename S>
    constexpr bool is_creatable_schema = !std::is_same_v<S, NoSchema>;

    template<typename S>
    constexpr bool is_named_schema = !(std::meta::has_template_arguments(^^S) && std::meta::template_of(^^S) == ^^Schema);

    template<typename S>
    constexpr bool is_namespace_scoped = std::meta::is_namespace(std::meta::parent_of(^^S));

    template<typename S>
    concept PlannableSchema = SchemaType<S> && is_creatable_schema<S> && is_named_schema<S> && is_namespace_scoped<S>;


    template<SchemaType S>
    consteval schema_plan_t plan_schema() {
        return {};
    }

    template<PlannableSchema S>
    consteval schema_plan_t plan_schema() {
        constexpr auto ns = std::meta::parent_of(^^S);
        static constexpr auto candidates = row_types_in(ns);

        const auto fail = [](const std::string& message) {
            return schema_plan_t{
                .entities = {},
                .error = std::define_static_string("Schema '" + std::string(S::name) + "': " + message)
            };
        };


        // Entities of this schema, their qualified names and what their foreign keys reference

        std::vector<std::meta::info> nodes;
        std::vector<std::string> names;
        std::vector<std::vector<std::string>> refs;

        template for (constexpr auto it : candidates) {
            using T = typename [:it:];

            if constexpr (Entity<T>) {
                if constexpr (std::is_same_v<schema_of<T>, S>) {
                    nodes.push_back(it);
                    names.emplace_back(qualified_name<T>);

                    auto& deps = refs.emplace_back();

                    template for (constexpr auto d : declaration_list<T>) {
                        for (const std::string_view table : entry_traits<entry_type<d>>::references)
                            deps.push_back(qualify(S::name, table));
                    }
                }
            }
        }

        size_t n = nodes.size();

        if (n == 0) {
            return fail("no entities found in namespace '" + std::string(std::meta::display_string_of(ns))
                + "' or its nested namespaces. Is every entity header included before this point?");
        }

        for (size_t i = 0; i < n; i++) {
            for (size_t j = i + 1; j < n; j++) {
                if (names[i] == names[j])
                    return fail("table '" + names[i] + "' is declared by more than one entity.");
            }
        }


        std::string prefix = std::string(S::name) + '.';

        std::vector<size_t> pending(n, 0);
        std::vector<std::vector<size_t>> dependants(n);

        for (size_t i = 0; i < n; i++) {
            for (const auto& ref : refs[i]) {
                if (!ref.starts_with(prefix) || ref == names[i])
                    continue;

                size_t j = 0;
                while (j < n && names[j] != ref)
                    j++;

                if (j == n)
                    return fail("foreign key in '" + names[i] + "' references unknown table '" + ref + "'.");

                pending[i]++;
                dependants[j].push_back(i);
            }
        }


        std::vector<std::meta::info> order;
        std::vector done(n, false);

        while (order.size() < n) {
            size_t next = 0;
            while (next < n && (done[next] || pending[next] != 0))
                next++;

            if (next == n) {
                std::string stuck;

                for (size_t i = 0; i < n; i++) {
                    if (done[i])
                        continue;

                    if (!stuck.empty())
                        stuck += ", ";

                    stuck += names[i];
                }

                return fail("foreign keys form a cycle, these tables cannot be ordered: " + stuck + '.');
            }

            done[next] = true;
            order.push_back(nodes[next]);

            for (const size_t it : dependants[next])
                pending[it]--;
        }

        return {
            .entities = std::define_static_array(order), 
            .error = {}
        };
    }


    template<SchemaType S>
    struct schema_plan {
        static_assert(is_creatable_schema<S>, "NoSchema has no tables to collect, declare a schema type.");

        static_assert(is_named_schema<S>,
            "Declare a named schema type, e.g. `struct Shop : conduit::reflect::Schema<\"shop\"> {};`, and use it "
            "in Table<> - the collector scans the namespace that type is declared in.");

        static_assert(is_namespace_scoped<S>, "A schema type must be declared at namespace scope.");

        static constexpr schema_plan_t value = plan_schema<S>();

        static_assert(value.error.empty(), value.error);
    };


    template<SchemaType S>
    constexpr std::span<const std::meta::info> entities_of = schema_plan<S>::value.entities;
}

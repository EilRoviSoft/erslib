#include "erslib/flux/query/layouts/insert.hpp"

// std
#include <utility>

// ers
#include <erslib/flux/query/clauses/assign.hpp>
#include <erslib/flux/query/clauses/conflict.hpp>
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/values.hpp>
#include <erslib/flux/query/slots/column.hpp>
#include <erslib/flux/query/slots/conflict.hpp>
#include <erslib/flux/query/slots/returning.hpp>
#include <erslib/flux/query/slots/set.hpp>
#include <erslib/flux/query/slots/values.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    ers::Status render_insert_set(Context& ctx, std::span<const Clause* const> group) {
        std::vector<const AssignClause*> assigns;
        assigns.reserve(group.size());

        for (const auto* clause : group) {
            const auto* assign = dynamic_cast<const AssignClause*>(&**clause);

            if (!assign) {
                ERS_TRY((*clause)->render(ctx));
                return ers::make_error("Only assignments are allowed in the SET of INSERT.");
            }

            assigns.push_back(assign);
        }

        ctx.query += " (";

        for (size_t i = 0; i < assigns.size(); ++i) {
            if (i != 0)
                ctx.query += ", ";

            ERS_TRY(assigns[i]->render_column(ctx));
        }

        ctx.query += ")\nVALUES (";

        for (size_t i = 0; i < assigns.size(); ++i) {
            if (i != 0)
                ctx.query += ", ";

            ERS_TRY(assigns[i]->render_value(ctx));
        }

        ctx.query += ")";
        return ers::ok;
    }
}

namespace {
    constexpr SlotRef set_excludes[] = { &slots::column, &slots::values };

    constexpr SlotBinding insert_layout[] = {
        {
            .slot   = &slots::insert_into,
            .prefix = "INSERT INTO "
        },
        {
            .slot     = &slots::set,
            .renderer = render_insert_set,
            .excludes = set_excludes
        },
        {
            .slot      = &slots::column,
            .prefix    = " (",
            .separator = ", ",
            .suffix    = ")"
        },
        bindings::values,
        bindings::conflict,
        bindings::conflict_constraint,
        bindings::conflict_action,
        bindings::returning,
    };
}


// Implementation --------------------------------------------------------------

QueryBuilder layouts::insert_into(std::string table) {
    QueryBuilder q(insert_layout);
    q |= clauses::identifier(&slots::insert_into, std::move(table));
    return q;
}

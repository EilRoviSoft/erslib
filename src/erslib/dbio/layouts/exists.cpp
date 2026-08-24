#include "erslib/dbio/layouts/exists.hpp"

// ers
#include <erslib/dbio/slots/exists.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


namespace {
    constexpr SlotBinding exists_layout[] = {
        {
            .slot   = &slots::exists_target,
            .prefix = "SELECT EXISTS (\nSELECT 1 FROM "
        },
        bindings::where,
        {
            .slot = &slots::exists_alias,
        }
    };
}

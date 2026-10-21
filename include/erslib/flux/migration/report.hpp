#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>


namespace flux::impl {
    enum class EMigrationEvent : u8 {
        Applied,
        Stamped,
        Skipped,
        Edited,
        Adopted,
        Accepted,
    };

    struct migration_event_t {
        EMigrationEvent kind;
        std::string subject;

        bool operator==(const migration_event_t&) const = default;
    };
    using migration_report_t = std::vector<migration_event_t>;
}

namespace flux::impl {
    constexpr std::string_view to_string(EMigrationEvent what) {
        switch (what) {
            case EMigrationEvent::Applied:
                return "applied";

            case EMigrationEvent::Stamped:
                return "stamped";

            case EMigrationEvent::Skipped:
                return "skipped";

            case EMigrationEvent::Edited:
                return "edited after being applied";

            case EMigrationEvent::Adopted:
                return "adopted";

            case EMigrationEvent::Accepted:
                return "declaration accepted";
        }

        return {};
    }
}

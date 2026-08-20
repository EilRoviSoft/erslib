#pragma once


namespace dbio::impl::sql {
    namespace kind {
        struct Select {};
        struct LoadByPk {};
        struct Insert {};
        struct Update {};
        struct Delete {};
        struct Save {};
    }

    namespace role {
        // Read from the result back into the entity.
        struct Read {};
        // Written into the statement as a value.
        struct Write {};
        // The tail clause's columns: WHERE terms for Update/Delete/LoadByPk, or the DO UPDATE SET assignments for Save.
        struct Key {};
    }
}

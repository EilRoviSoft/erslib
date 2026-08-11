#pragma once


namespace dbio::impl {
    enum PersistencyFlag : bool {
        // In  - reading entity fields out of a result row (SELECT ... -> entity)
        In = false,
        // Out - writing entity fields into a query (entity -> INSERT/UPDATE ...)
        Out = true
    };
}


// Exports

namespace dbio {
    using impl::PersistencyFlag;
    using impl::In;
    using impl::Out;
}

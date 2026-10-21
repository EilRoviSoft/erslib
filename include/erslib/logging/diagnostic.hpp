#pragma once

// quill
#include <quill/Logger.h>

// ers
#include <erslib/core/type/diagnostic.hpp>

// export
#include <erslib/export.hpp>


namespace ers::logging::impl {
    ERSLIB_EXPORT void log(quill::Logger* logger, const Diagnostic& what);
}

namespace ers::logging {
    using impl::log;
}

#pragma once

// quill
#include <quill/Logger.h>

// ers
#include <erslib/core/type/diagnostic.hpp>

// export
#include <erslib/export.hpp>


namespace ers::logging::impl {
    void ERSLIB_EXPORT log(quill::Logger* logger, const Diagnostic& what);
}

namespace ers::logging {
    using impl::log;
}

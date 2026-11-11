#pragma once

// std
#include <string>

// export
#include <erslib/export.hpp>


namespace ers::impl::text {
    ERSLIB_EXPORT std::string ru_to_en(std::string_view text);
}


// Exports ---------------------------------------------------------------------

namespace ers::text {
    using impl::text::ru_to_en;
}

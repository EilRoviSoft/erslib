#pragma once

// std
#include <string>


namespace ers::text::impl {
    std::string ru_to_en(std::string_view text);
}


// Exports ---------------------------------------------------------------------

namespace ers::text {
    using impl::ru_to_en;
}

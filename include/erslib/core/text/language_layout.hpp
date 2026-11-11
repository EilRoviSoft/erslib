#pragma once

// std
#include <string>


namespace ers::impl::text {
    std::string ru_to_en(std::string_view text);
}


// Exports ---------------------------------------------------------------------

namespace ers::text {
    using impl::text::ru_to_en;
}

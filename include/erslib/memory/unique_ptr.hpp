#pragma once


// boost
#include <memory>

namespace ers {
    template<typename T, typename Deleter = std::default_delete<T>>
    using unique_ptr = std::unique_ptr<T, Deleter>;
}

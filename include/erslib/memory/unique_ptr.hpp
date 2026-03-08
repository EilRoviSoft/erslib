#pragma once


// boost
#include <memory>


// Alias declaration

namespace ers {
    template<typename T, typename Deleter = std::default_delete<T>>
    using unique_ptr = std::unique_ptr<T, Deleter>;
}


// Function inclusion

namespace ers {
    using std::make_unique;
}

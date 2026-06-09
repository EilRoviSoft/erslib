#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/exception/internal.hpp>


namespace ers {
    ERS_MAKE_EXCEPTION_TYPE(path_error, std::runtime_error);
}

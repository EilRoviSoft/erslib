#pragma once

// std
#include <stdexcept>

// ers
#include <erslib/core/exception/impl.hpp>


namespace ers::impl {
    ERS_MAKE_EXCEPTION_TYPE(path_error, std::runtime_error);
}

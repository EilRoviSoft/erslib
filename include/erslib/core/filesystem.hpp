#pragma once

// std
#include <filesystem>

namespace fs = std::filesystem;


// Includes

#include <erslib/core/constant/filesystem.hpp>
#include <erslib/core/exception/filesystem_error.hpp>
#include <erslib/core/util/file.hpp>


// Exports

namespace ers {
    using impl::path_separator;

    ERS_MAKE_EXCEPTION_EXPORTS(impl, path_error);

    namespace util {
        using impl::util::read_file;
    }
}

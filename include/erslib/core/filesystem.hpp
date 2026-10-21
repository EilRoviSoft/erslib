#pragma once

// std
#include <filesystem>

#ifdef ERS_EXPOSE_GENERAL
namespace fs = std::filesystem;
#endif


// Includes --------------------------------------------------------------------

#include <erslib/core/constant/filesystem.hpp>
#include <erslib/core/exception/filesystem_error.hpp>
#include <erslib/core/memory/file.hpp>
#include <erslib/core/util/file.hpp>


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::path_separator;

    ERS_MAKE_EXCEPTION_EXPORTS(impl, path_error);

    using impl::file_ptr;

    namespace util {
        using impl::util::read_file;
    }
}

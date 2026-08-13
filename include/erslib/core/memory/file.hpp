#pragma once

// std
#include <cstdio>
#include <memory>


namespace ers::impl {
    struct FileCloser {
        void operator()(std::FILE* f) const noexcept { std::fclose(f); }
    };


    using file_ptr = std::unique_ptr<FILE, FileCloser>;
}

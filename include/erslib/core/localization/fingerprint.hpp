#pragma once

// std
#include <cstdint>
#include <string>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/localization/fwd.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    struct fingerprint_t {
        std::string path;
        std::uintmax_t size = 0;
        size_t files = 0;
        fs::file_time_type time = fs::file_time_type::min();
        size_t checksum = 0;


        // Nothing touched the folder - no need to read it.
        [[nodiscard]] bool same_metadata_as(const fingerprint_t& other) const noexcept {
            return size == other.size && files == other.files && time == other.time;
        }

        // The folder was touched, but its texts are the same.
        [[nodiscard]] bool same_content_as(const fingerprint_t& other) const noexcept {
            return size == other.size && files == other.files && checksum == other.checksum;
        }
    };
}

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
        size_t layout = 0;
        size_t checksum = 0;


        bool same_metadata_as(const fingerprint_t& other) const noexcept {
            return size == other.size && files == other.files && time == other.time && layout == other.layout;
        }

        bool same_content_as(const fingerprint_t& other) const noexcept {
            return size == other.size && files == other.files && checksum == other.checksum;
        }
    };
}

#pragma once

// std
#include <chrono>
#include <string>

// ers
#include <erslib/core/filesystem.hpp>


namespace test {
    // A fresh folder under the system temp dir, removed with everything inside.
    class TempTree {
    public:
        // Member functions

        explicit TempTree(std::string_view prefix = "ers_test");
        ~TempTree();

        TempTree(const TempTree&) = delete;
        TempTree& operator=(const TempTree&) = delete;


        // Accessors

        [[nodiscard]] const fs::path& root() const noexcept;
        [[nodiscard]] fs::path path(const fs::path& relative) const;

        [[nodiscard]] std::string read(const fs::path& relative) const;


        // Modifiers

        // Creates missing folders, replaces the file if it exists.
        void write(const fs::path& relative, std::string_view text) const;

        // Moves the file's time forward, so a change is seen even on filesystems with coarse clocks.
        void touch(const fs::path& relative, std::chrono::seconds by = std::chrono::seconds(2)) const;

        void remove(const fs::path& relative) const;


    private:
        fs::path _root;


        static fs::path make_root(std::string_view prefix);
    };
}

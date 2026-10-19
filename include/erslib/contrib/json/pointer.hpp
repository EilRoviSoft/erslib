#pragma once

// std
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace utl::impl {
    class ERSLIB_EXPORT JsonPointer {
    public:
        // Member functions

        JsonPointer() = default;
        explicit JsonPointer(std::string_view pointer);


        // Static methods

        [[nodiscard]]
        static std::optional<std::size_t> array_index(std::string_view token) noexcept;


        // Accessors

        [[nodiscard]]
        const std::vector<std::string>& tokens() const noexcept { return _tokens; }


        // Observers

        [[nodiscard]]
        bool empty() const noexcept { return _tokens.empty(); }


        // Generators

        [[nodiscard]]
        std::string to_string() const;


        // Operators overloading

        [[nodiscard]]
        bool operator==(const JsonPointer&) const noexcept = default;


    private:
        std::vector<std::string> _tokens;
    };
}

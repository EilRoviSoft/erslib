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
        JsonPointer() = default;
        explicit JsonPointer(std::string_view pointer);


        [[nodiscard]]
        static std::optional<std::size_t> array_index(std::string_view token) noexcept;


        [[nodiscard]]
        const std::vector<std::string>& tokens() const noexcept { return _tokens; }

        [[nodiscard]]
        bool empty() const noexcept { return _tokens.empty(); }

        [[nodiscard]]
        std::string to_string() const;

        [[nodiscard]]
        bool operator==(const JsonPointer&) const noexcept = default;


    private:
        std::vector<std::string> _tokens;
    };
}

#pragma once

// std
#include <string>

// pqxx
#include <pqxx/util>


namespace dbio {
    static constexpr std::string_view lower_digits = "0123456789abcdef";
    static constexpr std::string_view upper_digits = "0123456789ABCDEF";

    enum class Mode : uint8_t { HexLower, HexUpper, Text };

    std::string bytes_to_string(const pqxx::bytes& bytes, std::string_view args);
}

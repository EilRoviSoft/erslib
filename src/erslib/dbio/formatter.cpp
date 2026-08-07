#include "erslib/dbio/formatter.hpp"


std::string dbio::bytes_to_string(const pqxx::bytes& bytes, std::string_view args) {
    auto it = args.begin();
    auto mode = Mode::HexLower;

    if (it != args.end()) {
        switch (*it) {
            case 'x': mode = Mode::HexLower;
                ++it;
                break;

            case 'X': mode = Mode::HexUpper;
                ++it;
                break;

            case 's': mode = Mode::Text;
                ++it;
                break;

            default:
                break;
        }
    }

    std::string out;

    if (mode != Mode::Text) {
        const std::string_view digits = mode == Mode::HexLower ? lower_digits : upper_digits;
        out.reserve(bytes.size() * 2);

        for (auto byte : bytes) {
            auto v = std::to_integer<unsigned char>(byte);
            out.push_back(digits[v >> 4]);
            out.push_back(digits[v & 0xF]);
        }
    } else
        out.assign(reinterpret_cast<char const*>(bytes.data()), bytes.size());

    return out;
}

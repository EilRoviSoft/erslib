#include <erslib/core/convert/time.hpp>

// std
#include <format>
#include <sstream>


ers::Result<ers::timestamp_t> ers::impl::timestamp_from_local(
    std::string_view text,
    const std::chrono::time_zone& zone,
    std::string_view fmt
) {
    std::chrono::local_seconds local;

    std::istringstream in { std::string(text) };
    in >> std::chrono::parse(std::string(fmt), local);

    if (in.fail())
        return make_error("Can't parse local time '{}' with '{}'", text, fmt);

    return timestamp_t(zone.to_sys(local, std::chrono::choose::earliest));
}

std::string ers::impl::timestamp_to_local(
    timestamp_t what,
    const std::chrono::time_zone& zone,
    std::string_view fmt
) {
    const auto local = zone.to_local(std::chrono::floor<std::chrono::seconds>(what));

    return std::vformat(std::format("{{:{}}}", fmt), std::make_format_args(local));
}

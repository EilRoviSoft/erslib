#include "erslib/modkit/util/stage.hpp"

// boost
#include <boost/regex.hpp>

// std
#include <charconv>


namespace {
    constexpr auto stage_name_pattern = R"((.+)-(\d+))";
}


bool ers::modkit::impl::is_stage_naming_scheme(std::string_view sv) {
    boost::regex regex(stage_name_pattern);
    return boost::regex_match(sv.begin(), sv.end(), regex);
}

std::tuple<std::string, size_t> ers::modkit::impl::extract_stage_info(std::string_view sv) {
    boost::regex regex(stage_name_pattern);
    boost::match_results<std::string_view::const_iterator> match;

    if (!boost::regex_match(sv.begin(), sv.end(), match, regex) || match.size() != 3)
        return {};

    const std::string digits = match[2].str();

    size_t index = 0;
    if (std::from_chars(digits.data(), digits.data() + digits.size(), index).ec != std::errc {})
        return {};

    return { match[1], index };
}

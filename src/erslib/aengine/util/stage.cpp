#include "erslib/aengine/util/stage.hpp"

// boost
#include <boost/regex.hpp>

// ers
#include <erslib/core/convert/string.hpp>


namespace {
    constexpr auto stage_name_pattern = R"((.+)-(\d+))";
}


bool aengine::util::is_stage_naming_scheme(std::string_view sv) {
    boost::regex regex(stage_name_pattern);
    return boost::regex_match(sv.begin(), sv.end(), regex);
}

std::tuple<std::string, size_t> aengine::util::extract_stage_info(std::string_view sv) {
    boost::regex regex(stage_name_pattern);
    boost::match_results<std::string_view::const_iterator> match;

    if (!boost::regex_match(sv.begin(), sv.end(), match, regex) || match.size() != 3)
        return {};

    auto r = ers::convert::from_str<size_t>(match[2].str());
    if (!r)
        return {};

    return { match[1], *r };
}

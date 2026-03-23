#pragma once

// std
#include <string>

// ers
#include <erslib/adaptor/transparent/base.hpp>


namespace ers {
    template<typename Policy>
    using string_hash_adaptor = hash_adaptor<Policy, std::string, std::string_view, const char*>;

    template<typename Policy, auto Member>
    using member_string_hash_adaptor = member_hash_adaptor<Policy, Member, std::string, std::string_view, const char*>;
}

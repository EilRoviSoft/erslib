#pragma once

// std
#include <string>

// ers
#include <erslib/core/adaptor/transparent_base.hpp>


#define ERS_STRING_LIKE_TYPES_PACK std::string, std::string_view, const char*


namespace ers::impl {
    template<template<typename> typename Hash>
    using string_hash = hash<Hash, ERS_STRING_LIKE_TYPES_PACK>;

    template<template<typename> typename Hash, auto Member>
    using member_string_hash = member_hash<Hash, Member, ERS_STRING_LIKE_TYPES_PACK>;
}

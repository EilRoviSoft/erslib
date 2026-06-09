#pragma once

// std
#include <string>

// ers
#include <erslib/adaptor/transparent/base.hpp>


#define ERS_STRING_LIKE_TYPES_PACK std::string, std::string_view, const char*


namespace ers {
    template<template<typename> typename Hash>
    using string_hash_adaptor = hash_adaptor<Hash, ERS_STRING_LIKE_TYPES_PACK>;

    template<template<typename> typename Hash, auto Member>
    using member_string_hash_adaptor = member_hash_adaptor<Hash, Member, ERS_STRING_LIKE_TYPES_PACK>;
}

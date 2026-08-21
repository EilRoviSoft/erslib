#pragma once

// std
#include <cmath>
#include <cstdint>


namespace ers::impl::type {
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float_t;
    using f64 = double_t;
}

namespace ers::impl::type {
    template<typename T>
    struct vec2 {
        T x;
        T y;
    };
}


// Exports

namespace ers::type {
    using impl::type::u8;
    using impl::type::u16;
    using impl::type::u32;
    using impl::type::u64;

    using impl::type::i8;
    using impl::type::i16;
    using impl::type::i32;
    using impl::type::i64;

    using impl::type::f32;
    using impl::type::f64;


    using impl::type::vec2;
}


#ifndef REMOVE_ERS_GENERAL_TYPES
using namespace ers::type;
#endif

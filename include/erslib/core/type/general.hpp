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


// Exports

namespace ers {
    namespace type = impl::type;
}


#ifndef REMOVE_ERS_GENERAL_TYPES
using namespace ers::type;
#endif

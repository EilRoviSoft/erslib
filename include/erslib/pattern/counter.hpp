#pragma once

namespace ers::pattern {
    template<typename>
    static size_t global_counter() {
        static size_t i = 0;
        return i++;
    }
}

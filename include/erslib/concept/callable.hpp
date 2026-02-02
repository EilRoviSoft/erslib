#pragma once

namespace ers {
    template<typename T>
    concept Callable = requires { &T::operator(); };
}

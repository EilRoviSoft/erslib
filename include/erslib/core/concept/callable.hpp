#pragma once


namespace ers::impl {
    template<typename T>
    concept Callable = requires { &T::operator(); };
}

#pragma once


// Functions

namespace ers::hashing {
    template<template<typename> typename HashEngine, typename... Args>
    constexpr size_t combine(const Args&... args) noexcept {
        size_t r = 0;

        auto combine_step = [&r]<typename T>(const T& arg) {
            r ^= HashEngine<T>(arg) + 0x9e3779b97f4a7c15 + (r << 6) + (r >> 2);
        };

        (combine_step(args), ...);

        return r;
    }
}

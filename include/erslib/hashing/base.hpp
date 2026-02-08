#pragma once

// ers
#include <erslib/trait/byte_layout.hpp>

namespace ers::hashing::_impl {
    // To work properly, you should override only 1 per algo
    template<typename Policy>
    struct backend {
        static constexpr size_t process(
            const void* data,
            size_t size,
            size_t seed
        ) noexcept = delete("non-specialized definition");

        template<typename T>
        static constexpr size_t process_value(
            const T& data,
            size_t seed
        ) noexcept = delete("non-specialized definition");
    };
}

namespace ers {
    template<typename T, typename Tag>
    struct THashBase {
        constexpr size_t operator()(const T& what, size_t seed = 0) const noexcept {
            if constexpr (requires { hashing::_impl::backend<Tag>::process(what, seed); }) {
                return hashing::_impl::backend<Tag>::process_value(what, seed);
            } else {
                return hashing::_impl::backend<Tag>::process(
                    byte_layout_traits<T>::data(what),
                    byte_layout_traits<T>::size(what),
                    seed
                );
            }
        }
    };
}

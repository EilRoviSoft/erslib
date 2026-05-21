#pragma once

// std
#include <ranges>


namespace ers::adaptor {
    template<typename MemPtr, typename ViewFn>
    struct member_range {
        member_range(MemPtr mem_ptr, ViewFn view_fn) :
            _mem_ptr(mem_ptr),
            _view_fn(view_fn) {
        }

        template<std::ranges::viewable_range R>
        friend auto operator|(R&& range, member_range adaptor) {
            return adaptor._view_fn(
                std::forward<R>(range),
                [mp = adaptor._mem_ptr]<typename T>(T&& obj) -> decltype(auto) {
                    return std::invoke(mp, std::forward<T>(obj));
                }
            );
        }


    private:
        MemPtr _mem_ptr;
        ViewFn _view_fn;
    };
}

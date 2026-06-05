#pragma once

// std
#include <ranges>
#include <optional>
#include <functional>


// filer_view

namespace ers::internal {
    template<std::ranges::forward_range R, typename F>
        requires std::ranges::view<R> && std::predicate<F, std::ranges::range_reference_t<R>>
    struct filter_view : std::ranges::view_interface<filter_view<R, F>> {
    public:
        filter_view() = default;

        constexpr filter_view(R range, F fn) :
            _range(std::move(range)),
            _fn(std::move(fn)) {
        }


        constexpr auto begin() const {
            if (!_cached_begin)
                _cached_begin = std::ranges::find_if(_range, _fn);
            return *_cached_begin;
        }

        constexpr auto end() const {
            return std::ranges::end(_range);
        }


    private:
        R _range;
        F _fn;
        mutable std::optional<std::ranges::iterator_t<R>> _cached_begin;
    };


    template<typename R, typename F>
    filter_view(R&&, F) -> filter_view<std::views::all_t<R>, F>;
}


// filter_closure

namespace ers::internal {
    template<typename F>
    struct filter_closure : std::ranges::range_adaptor_closure<filter_closure<F>> {
        constexpr explicit filter_closure(F fn) :
            _fn(std::move(fn)) {
        }


        template<std::ranges::viewable_range R>
        constexpr auto operator()(R&& range) const {
            return filter_view(std::views::all(std::forward<R>(range)), _fn);
        }


    private:
        F _fn;
    };
}


// filter_fn

namespace ers::internal {
    struct filter_fn {
        template<typename F>
        constexpr auto operator()(F&& fn) const {
            return filter_closure<std::decay_t<F>>(std::forward<F>(fn));
        }

        template<std::ranges::viewable_range R, typename F>
        constexpr auto operator()(R&& range, F&& fn) const {
            return filter_view(
                std::views::all(std::forward<R>(range)),
                std::forward<F>(fn)
            );
        }
    };
}


namespace ers::views {
    inline constexpr internal::filter_fn filter;
}

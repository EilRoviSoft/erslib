#pragma once


// Includes

#include <erslib/core/concept/callable.hpp>
#include <erslib/core/concept/constexpr.hpp>
#include <erslib/core/concept/container.hpp>
#include <erslib/core/concept/functional.hpp>
#include <erslib/core/concept/pair.hpp>
#include <erslib/core/concept/sbo.hpp>
#include <erslib/core/concept/string.hpp>
#include <erslib/core/concept/util.hpp>


// Exports

namespace ers {
    using impl::Callable;

    using impl::is_constexpr_friendly;
    using impl::is_constexpr_friendly_v;

    using impl::ContiguousContainer;
    using impl::HashMapConcept;

    using impl::UnaryWith;
    using impl::BinaryWith;
    using impl::ExecutableWith;
    using impl::HashableWith;
    using impl::EqualWith;

    using impl::PairLike;

    using impl::is_sbo_applicable;
    using impl::is_sbo_applicable_v;
    using impl::ESboPolicy;

    using impl::StringConvertible;

    using impl::SameAsIn;
    using impl::ConvertibleToIn;
}

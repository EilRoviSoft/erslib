#pragma once


// Original work Copyright (c) anthonywilliams
// Distributed under the Boost Software License, Version 1.0.
// Modifications Copyright (c) 2026 eilrovisoft


// std
#include <type_traits>
#include <utility>
#include <functional>
#include <ostream>


namespace contrib::detail {
    // A small type used for operation tests
    using small_result_t = char;

    // A large type with a different size to the small type, used for
    // operation tests
    struct large_result_t {
        small_result_t dummy[2];
    };
}


namespace contrib {
    // The StrongTypedef template used to create unique types with
    // specific properties.
    //
    // Tag is a tag type used for uniqueness. It can be incomplete as
    // it is never used for anything except creating unique template
    // instantiations
    //
    // ValueType is the type this Strong typedef is based on: it is the type of
    // the underlying value
    //
    // Properties are types that provide mixins that enable certain operations
    // on values of this type.
    template<typename Tag, typename ValueType, typename... Properties>
    class StrongTypedef : public Properties::template mixin<StrongTypedef<Tag, ValueType, Properties...>, ValueType>... {
    public:
        // The underlying value type
        using underlying_value_type = ValueType;

        // A default constructed StrongTypedef has a default-constructed value
        constexpr StrongTypedef() noexcept : value() {}

        // Construct a StrongTypedef holding the specified value
        explicit constexpr StrongTypedef(ValueType value_) noexcept(std::is_nothrow_move_constructible_v<ValueType>) :
            value(std::move(value_)) {
        }

        // Explicit conversion operator to read the underlying value
        explicit constexpr operator ValueType const&() const noexcept {
            return value;
        }

        // Get a const reference to the underlying value
        constexpr ValueType const& underlying_value() const noexcept {
            return value;
        }

        // Get a reference to the underlying value
        constexpr ValueType& underlying_value() noexcept {
            return value;
        }

        // Get a reference to the underlying value
        friend constexpr ValueType& underlying_value(StrongTypedef& t) {
            return t.underlying_value();
        }

        // Get a const reference to the underlying value
        friend constexpr ValueType const& underlying_value(StrongTypedef const& t) {
            return t.underlying_value();
        }

        // Get a const rvalue reference to the underlying value
        friend constexpr ValueType&& underlying_value(StrongTypedef&& t) {
            return std::move(t.underlying_value());
        }

        // Get an rvalue reference to the underlying value
        friend constexpr ValueType const&& underlying_value(StrongTypedef const&& t) {
            return std::move(t.underlying_value());
        }


    private:
        // The underlying value
        ValueType value;
    };


    // The underlying value of a value that is not a StrongTypedef is just
    // that value
    template<typename T>
    constexpr T&& underlying_value(T&& t) {
        return std::forward<T>(t);
    }
}


namespace contrib::strong_typedef_properties {
    // Add operator== to the StrongTypedef
    struct equality_comparable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr bool operator==(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() == std::declval<ValueType const&>())) {
                return lhs.underlying_value() == rhs.underlying_value();
            }
        };
    };

    // Add the pre increment operator to the StrongTypedef
    struct pre_incrementable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived& operator++(
                Derived& self
            ) noexcept(noexcept(++std::declval<ValueType&>())) {
                ++self.underlying_value();
                return self;
            }
        };
    };

    // Add the post-increment operator to the StrongTypedef
    struct post_incrementable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived operator++(
                Derived& self, int
            ) noexcept(noexcept(std::declval<ValueType&>()++)) {
                return Derived { self.underlying_value()++ };
            }
        };
    };

    // Add both pre- and post-increment operators to the StrongTypedef
    struct incrementable {
        template<typename Derived, typename ValueType>
        struct mixin : pre_incrementable::mixin<Derived, ValueType>,
            post_incrementable::mixin<Derived, ValueType> {
        };
    };

    // Add the pre-decrement operator to the StrongTypedef
    struct pre_decrementable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived& operator--(
                Derived& self
            ) noexcept(noexcept(--std::declval<ValueType&>())) {
                --self.underlying_value();
                return self;
            }
        };
    };

    // Add the post-decrement operator to the StrongTypedef
    struct post_decrementable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived operator--(
                Derived& self, int
            ) noexcept(noexcept(std::declval<ValueType&>()--)) {
                return Derived { self.underlying_value()-- };
            }
        };
    };

    // Add both pre- and post-decrement operators to the StrongTypedef
    struct decrementable {
        template<typename Derived, typename ValueType>
        struct mixin : pre_decrementable::mixin<Derived, ValueType>, post_decrementable::mixin<Derived, ValueType> {};
    };

    // Add operator+ that supports adding the StrongTypedef to anything
    // the underlying value can be added to
    struct generic_mixed_addable {
        template<typename Derived, typename ValueType>
        struct mixin {
            template<typename Rhs>
            friend constexpr Derived operator+(
                Derived const& lhs, Rhs const& rhs
            ) noexcept(noexcept(
                std::declval<ValueType const&>()
                + underlying_value(std::declval<Rhs const&>()))) requires (!std::is_same_v<Rhs, Derived>
                && std::is_convertible_v<decltype(std::declval<ValueType const&>() + underlying_value(std::declval<Rhs const&>())), ValueType>
            ) {
                return Derived {
                    lhs.underlying_value() +
                    underlying_value(rhs)
                };
            }

            template<typename Lhs>
            friend constexpr Derived operator+(
                Lhs const& lhs, Derived const& rhs
            ) noexcept(noexcept(
                underlying_value(std::declval<Lhs const&>())
                + std::declval<ValueType const&>())) requires (!std::is_same_v<Lhs, Derived>
                && std::is_convertible_v<decltype(underlying_value(std::declval<Lhs const&>()) + std::declval<ValueType const&>()), ValueType>
            ) {
                return Derived {
                    underlying_value(lhs) +
                    rhs.underlying_value()
                };
            }
        };
    };

#define JSS_COMPOUND_ASSIGN(op_symbol) op_symbol##=

#define JSS_DEFINE_OP_MIXINS(name, op_symbol)                                  \
    /** Add operator op_symbol to the StrongTypedef **/                        \
    template <typename Other> struct mixed_##name {                            \
        template <                                                             \
            typename Derived, typename ValueType>                              \
        struct mixin {                                                         \
            friend constexpr Derived                                           \
            operator op_symbol(Derived const &lhs, Other const &rhs) noexcept( \
                noexcept(std::declval<ValueType const &>()                     \
                             op_symbol underlying_value(                       \
                                 std::declval<Other const &>()))) {            \
                return Derived{lhs.underlying_value()                          \
                                   op_symbol underlying_value(rhs)};           \
            }                                                                  \
                                                                               \
            friend constexpr Derived                                           \
            operator op_symbol(Other const &lhs, Derived const &rhs) noexcept( \
                noexcept(underlying_value(std::declval<Other const &>())       \
                             op_symbol std::declval<ValueType const &>())) {   \
                return Derived{underlying_value(lhs)                           \
                                   op_symbol rhs.underlying_value()};          \
            }                                                                  \
                                                                               \
            friend constexpr Derived &operator JSS_COMPOUND_ASSIGN(op_symbol)( \
                Derived &lhs,                                                  \
                Other const                                                    \
                    &rhs) noexcept(noexcept(std::declval<ValueType &>()        \
                                                JSS_COMPOUND_ASSIGN(op_symbol) \
                                                    underlying_value(          \
                                                        std::declval<          \
                                                            Other const        \
                                                                &>()))) {      \
                lhs.underlying_value() JSS_COMPOUND_ASSIGN(op_symbol)          \
                    underlying_value(rhs);                                     \
                return lhs;                                                    \
            }                                                                  \
        };                                                                     \
    };                                                                         \
                                                                               \
    struct self_##name {                                                       \
        template <                                                             \
            typename Derived, typename ValueType>                              \
        struct mixin {                                                         \
            friend constexpr Derived operator op_symbol(                       \
                Derived const &lhs,                                            \
                Derived const                                                  \
                    &rhs) noexcept(noexcept(std::declval<ValueType const &>()  \
                                                op_symbol std::declval<        \
                                                    ValueType const &>())) {   \
                return Derived{lhs.underlying_value()                          \
                                   op_symbol rhs.underlying_value()};          \
            }                                                                  \
            friend constexpr Derived &operator JSS_COMPOUND_ASSIGN(op_symbol)( \
                Derived &lhs,                                                  \
                Derived const                                                  \
                    &rhs) noexcept(noexcept(std::declval<ValueType &>()        \
                                                JSS_COMPOUND_ASSIGN(op_symbol) \
                                                    std::declval<              \
                                                        ValueType const        \
                                                            &>())) {           \
                lhs.underlying_value() JSS_COMPOUND_ASSIGN(op_symbol)          \
                    rhs.underlying_value();                                    \
                return lhs;                                                    \
            }                                                                  \
        };                                                                     \
    };                                                                         \
                                                                               \
    struct name {                                                              \
        template <typename Derived, typename ValueType>                        \
        struct mixin                                                           \
            : self_##name::mixin<Derived, ValueType>,                          \
              mixed_##name<ValueType>::template mixin<Derived, ValueType> {};  \
    };

    // Define mixins for the built-in operators
    //
    // self_xxx provides the operation where both LHS and RHS are
    // the StrongTypedef
    //
    // mixed_xxx<Other> provides the operation where only the LHS or RHS
    // is the StrongTypedef and the other operand is of type Other
    //
    // xxx combines self_xxx and mixed_xxx<underlying_value_type>
    //
    JSS_DEFINE_OP_MIXINS(addable, +)

    JSS_DEFINE_OP_MIXINS(subtractable, -)

    JSS_DEFINE_OP_MIXINS(multiplicable, *)

    JSS_DEFINE_OP_MIXINS(divisible, /)

    JSS_DEFINE_OP_MIXINS(modulus, %)

    JSS_DEFINE_OP_MIXINS(bitwise_or, |)

    JSS_DEFINE_OP_MIXINS(bitwise_and, &)

    JSS_DEFINE_OP_MIXINS(bitwise_xor, ^)

    // Allow subtraction with any type that can be subtracted from the
    // underlying value, or the underlying value can be subtracted from
    struct generic_mixed_subtractable {
        template<typename Derived, typename ValueType>
        struct mixin {
            template<typename Rhs>
            friend constexpr Derived operator-(
                Derived const& lhs, Rhs const& rhs
            ) noexcept(noexcept(
                std::declval<ValueType const&>()
                - underlying_value(std::declval<Rhs const&>()))) requires (!std::is_same_v<Rhs, Derived>
                && std::is_convertible_v<decltype(std::declval<ValueType const&>() - underlying_value(std::declval<Rhs const&>())), ValueType>
            ) {
                return Derived {
                    lhs.underlying_value() -
                    underlying_value(rhs)
                };
            }

            template<typename Lhs>
            friend constexpr Derived operator-(
                Lhs const& lhs, Derived const& rhs
            ) noexcept(noexcept(
                underlying_value(std::declval<Lhs const&>())
                - std::declval<ValueType const&>())) requires (!std::is_same_v<Lhs, Derived>
                && std::is_convertible_v<decltype(underlying_value(std::declval<Lhs const&>()) - std::declval<ValueType const&>()), ValueType>
            ) {
                return Derived {
                    underlying_value(lhs) -
                    rhs.underlying_value()
                };
            }
        };
    };

    // Allow subtracting two StrongTypedef instances to produce
    // a DifferenceType value tha represents the difference
    template<typename DifferenceType>
    struct difference {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr DifferenceType operator-(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() - std::declval<ValueType const&>())) {
                return DifferenceType {
                    lhs.underlying_value() -
                    rhs.underlying_value()
                };
            }
        };
    };

    // Add ordering comparison operators to the StrongTypedef
    struct ordered {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr bool operator<(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() < std::declval<ValueType const&>())) {
                return lhs.underlying_value() < rhs.underlying_value();
            }
            friend constexpr bool operator>(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() > std::declval<ValueType const&>())) {
                return lhs.underlying_value() > rhs.underlying_value();
            }
            friend constexpr bool operator<=(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() <= std::declval<ValueType const&>())) {
                return lhs.underlying_value() <= rhs.underlying_value();
            }
            friend constexpr bool operator>=(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(std::declval<ValueType const&>() >= std::declval<ValueType const&>())) {
                return lhs.underlying_value() >= rhs.underlying_value();
            }
        };
    };

    // Add ordering comparisons to the StrongTypedef where the
    // other operand is of type Other
    template<typename Other>
    struct mixed_ordered {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr auto operator<=>(
                Derived const& lhs, Derived const& rhs
            ) noexcept(noexcept(lhs.underlying_value() <=> underlying_value(rhs))
            ) requires(
                !std::same_as<Other, Derived>
                && std::three_way_comparable_with<ValueType, decltype(underlying_value(std::declval<Other const&>()))>
            ) {
                return lhs.underlying_value() <=> underlying_value(rhs);
            }
        };
    };

    
    // Allow this StrongTypedef to be used with std::hash
    struct hashable {
        struct base {};

        template<typename Derived, typename ValueType>
        struct mixin : base {};
    };

    // Add a stream operator to write the StrongTypedef to a std::ostream
    struct streamable {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend std::ostream& operator<<(std::ostream& os, Derived const& st) {
                return os << st.underlying_value();
            }
        };
    };

    // Combine ordered and equality_comparable
    struct comparable {
        template<typename Derived, typename ValueType>
        struct mixin : ordered::template mixin<Derived, ValueType>, equality_comparable::template mixin<Derived, ValueType> {};
    };

    // Add a division operation to the StrongTypedef that
    // produces a RatioType instances representing the result
    template<typename RatioType>
    struct ratio {
        template<typename Derived, typename ValueType>
        struct mixin {
            friend constexpr RatioType operator/(
                Derived const& lhs, Derived const& rhs
            ) noexcept(
                noexcept(
                    std::declval<ValueType const&>() /
                    std::declval<ValueType const&>())) {
                return RatioType {
                    lhs.underlying_value() /
                    rhs.underlying_value()
                };
            }
        };
    };

    // Add the bitwise not operator to the StrongTypedef
    struct bitwise_not {
        template<
            typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived operator~(
                Derived const& lhs
            ) noexcept(
                noexcept(~std::declval<ValueType const&>())) {
                return Derived { ~lhs.underlying_value() };
            }
        };
    };

    // Add the bitwise left-shift operator to the StrongTypedef
    template<typename Other>
    struct bitwise_left_shift {
        template<
            typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived operator<<(
                Derived const& lhs, Other const& rhs
            ) noexcept(
                noexcept(
                    std::declval<ValueType const&>()
                    << underlying_value(std::declval<Other const&>()))) {
                return Derived {
                    lhs.underlying_value()
                    << underlying_value(rhs)
                };
            }
            friend constexpr Derived& operator<<=(
                Derived& lhs, Other const& rhs
            ) noexcept(noexcept(
                std::declval<ValueType&>() <<=
                underlying_value(std::declval<Other const&>()))) {
                lhs.underlying_value() <<= underlying_value(rhs);
                return lhs;
            }
        };
    };

    // Add the bitwise right-shift operator to the StrongTypedef
    template<typename Other>
    struct bitwise_right_shift {
        template<
            typename Derived, typename ValueType>
        struct mixin {
            friend constexpr Derived operator>>(
                Derived const& lhs, Other const& rhs
            ) noexcept(
                noexcept(
                    std::declval<ValueType const&>() >>
                    underlying_value(std::declval<Other const&>()))) {
                return Derived {
                    lhs.underlying_value() >>
                    underlying_value(rhs)
                };
            }
            friend constexpr Derived& operator>>=(
                Derived& lhs, Other const& rhs
            ) noexcept(noexcept(
                std::declval<ValueType&>() >>=
                underlying_value(std::declval<Other const&>()))) {
                lhs.underlying_value() >>= underlying_value(rhs);
                return lhs;
            }
        };
    };
} // namespace strong_typedef_properties


// A specialization of std::hash for those instances of StrongTypedef that
// have the hashable property
template<typename Tag, typename ValueType, typename... Properties>
struct std::hash<contrib::StrongTypedef<Tag, ValueType, Properties...>> {
    template<typename Arg>
    constexpr
    size_t operator()(
        Arg const& arg
    ) const noexcept(noexcept(std::hash<ValueType>()(std::declval<ValueType const&>()))
    ) requires (
        std::is_same_v<Arg, contrib::StrongTypedef<Tag, ValueType, Properties...>>
        && std::is_base_of_v<contrib::strong_typedef_properties::hashable::base, Arg>
    ) {
        return std::hash<ValueType>()(arg.underlying_value());
    }
};

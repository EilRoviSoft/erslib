#pragma once

// std
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/ref.hpp>


// Forward decl

namespace ers::impl {
    template<typename T>
    class Result;


    struct ok_t {};

    constexpr ok_t ok;


    template<typename T>
    struct is_result : std::false_type {};

    template<typename T>
    struct is_result<Result<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_result_v = is_result<T>::value;
}


// Monadic operations

namespace ers::impl {
    struct ResultMonad {
        template<typename Self, typename F>
        constexpr auto and_then(this Self&& self, F&& f) {
            using R = std::remove_cvref_t<decltype(_call(std::forward<Self>(self), std::forward<F>(f)))>;

            static_assert(is_result_v<R>, "and_then: the callable must return ers::Result");

            if (!self)
                return R(_error(std::forward<Self>(self)));

            return R(_call(std::forward<Self>(self), std::forward<F>(f)));
        }

        template<typename Self, typename F>
        constexpr auto transform(this Self&& self, F&& f) {
            using U = std::remove_cvref_t<decltype(_call(std::forward<Self>(self), std::forward<F>(f)))>;

            if (!self)
                return Result<U>(_error(std::forward<Self>(self)));

            if constexpr (std::is_void_v<U>) {
                _call(std::forward<Self>(self), std::forward<F>(f));
                return Result<U>(ok);
            } else
                return Result<U>(_call(std::forward<Self>(self), std::forward<F>(f)));
        }

        template<typename Self, typename F>
        constexpr auto or_else(this Self&& self, F&& f) {
            using R = std::remove_cvref_t<Self>;

            if (self)
                return R(std::forward<Self>(self));

            return R(std::invoke(std::forward<F>(f), _error(std::forward<Self>(self))));
        }

        template<typename Self, typename U>
        constexpr auto value_or(this Self&& self, U&& fallback)
            requires (!std::is_void_v<typename std::remove_cvref_t<Self>::value_type>) {
            using T = typename std::remove_cvref_t<Self>::value_type;

            return self ? T(std::forward<Self>(self).value()) : T(std::forward<U>(fallback));
        }


    private:
        template<typename Self, typename F>
        static constexpr decltype(auto) _call(Self&& self, F&& f) {
            if constexpr (std::is_void_v<typename std::remove_cvref_t<Self>::value_type>)
                return std::invoke(std::forward<F>(f));
            else
                return std::invoke(std::forward<F>(f), std::forward<Self>(self).value());
        }

        template<typename Self>
        static constexpr Diagnostic _error(Self&& self) {
            if constexpr (std::is_rvalue_reference_v<Self&&> && !std::is_const_v<std::remove_reference_t<Self>>)
                return std::move(self).extract_error();
            else
                return self.error();
        }
    };
}


// Result<T>

namespace ers::impl {
    template<typename T>
    class [[nodiscard]] Result : public ResultMonad {
    public:
        using value_type = T;
        using error_type = Diagnostic;


        static_assert(!std::is_rvalue_reference_v<value_type>,
            "Result<T&&> is not allowed. Use Result<T> instead.");


        // Constructors

        constexpr Result()
            requires std::is_default_constructible_v<value_type> :
            m_variant(T {}) {
        }

        constexpr Result(const value_type& value) :
            m_variant(value) {
        }

        constexpr Result(value_type&& value) noexcept :
            m_variant(std::move(value)) {
        }

        constexpr Result(const error_type& error) :
            m_variant(std::in_place_index<1>, error) {
        }

        constexpr Result(error_type&& error) :
            m_variant(std::in_place_index<1>, std::move(error)) {
        }

        constexpr Result(const Result&) = default;
        constexpr Result(Result&&) = default;

        constexpr Result& operator=(const Result&) = default;
        constexpr Result& operator=(Result&&) = default;

        constexpr Result& operator=(const value_type& value) {
            m_variant = value;
            return *this;
        }
        constexpr Result& operator=(value_type&& value) {
            m_variant = std::move(value);
            return *this;
        }

        constexpr Result& operator=(const error_type& error) {
            m_variant = error;
            return *this;
        }
        constexpr Result& operator=(error_type&& error) {
            m_variant = std::move(error);
            return *this;
        }


        // Observers

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::holds_alternative<value_type>(m_variant); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::holds_alternative<error_type>(m_variant); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }


        // Accessors

        constexpr value_type& value() & { return std::get<0>(m_variant); }
        constexpr const value_type& value() const & { return std::get<0>(m_variant); }
        constexpr value_type&& value() && { return std::move(std::get<0>(m_variant)); }
        constexpr const value_type&& value() const && { return std::move(std::get<0>(m_variant)); }

        constexpr value_type& operator*() & { return value(); }
        constexpr const value_type& operator*() const & { return value(); }
        constexpr value_type&& operator*() && { return std::move(value()); }
        constexpr const value_type&& operator*() const && { return std::move(value()); }

        constexpr value_type* operator->() { return &value(); }
        constexpr const value_type* operator->() const { return &value(); }

        constexpr const error_type& error() const { return std::get<1>(m_variant); }


        // Modifiers

        // Needed just in case you want to extend error's message
        constexpr error_type&& extract_error() && { return std::move(std::get<1>(m_variant)); }


    protected:
        std::variant<value_type, error_type> m_variant;
    };
}


// Result<T&>

namespace ers::impl {
    template<typename T>
    class [[nodiscard]] Result<T&> : public ResultMonad {
    public:
        using value_type = T;
        using error_type = Diagnostic;


        // Constructors

        constexpr Result()
            requires std::is_default_constructible_v<value_type> :
            m_variant(value_type {}) {
        }

        constexpr Result(const value_type& value) :
            m_variant(value) {
        }

        constexpr Result(value_type&& value) = delete;

        constexpr Result(const error_type& error) :
            m_variant(std::in_place_index<1>, error) {
        }

        constexpr Result(error_type&& error) :
            m_variant(std::in_place_index<1>, std::move(error)) {
        }

        constexpr Result(const Result&) = default;
        constexpr Result(Result&&) = default;

        constexpr Result& operator=(const Result&) = default;
        constexpr Result& operator=(Result&&) = default;


        constexpr Result& operator=(const value_type& value) {
            m_variant = value;
            return *this;
        }
        constexpr Result& operator=(value_type&& value) = delete;

        constexpr Result& operator=(const error_type& error) {
            m_variant = error;
            return *this;
        }
        constexpr Result& operator=(error_type&& error) {
            m_variant = std::move(error);
            return *this;
        }


        // Observers

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::holds_alternative<ref<value_type>>(m_variant); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::holds_alternative<error_type>(m_variant); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }


        // Accessors

        constexpr value_type& value() & { return *std::get<0>(m_variant); }
        constexpr const value_type& value() const & { return *std::get<0>(m_variant); }
        constexpr value_type&& value() && { return std::move(*std::get<0>(m_variant)); }
        constexpr const value_type&& value() const && { return std::move(*std::get<0>(m_variant)); }

        constexpr value_type& operator*() & { return value(); }
        constexpr const value_type& operator*() const & { return value(); }
        constexpr value_type&& operator*() && { return std::move(value()); }
        constexpr const value_type&& operator*() const && { return std::move(value()); }

        constexpr value_type* operator->() { return &value(); }
        constexpr const value_type* operator->() const { return &value(); }

        constexpr const error_type& error() const { return std::get<1>(m_variant); }


        // Modifiers

        // Needed just in case you want to extend error's message
        constexpr error_type&& extract_error() && { return std::move(std::get<1>(m_variant)); }


    protected:
        std::variant<ref<value_type>, error_type> m_variant;
    };
}


// Template deduction

namespace ers::impl {
    template<typename T>
    Result(T) -> Result<T>;

    template<typename T>
    constexpr void swap(Result<T>& lhs, Result<T>& rhs) noexcept(noexcept(std::swap(lhs.m_variant, rhs.m_variant))) {
        std::swap(lhs.m_variant, rhs.m_variant);
    }

    template<typename T>
    constexpr bool operator==(const Result<T>& lhs, const Result<T>& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;

        if (lhs.has_value())
            return lhs.value() == rhs.value();

        return lhs.error() == rhs.error();
    }
}


// Result<void, ...>

namespace ers::impl {
    template<>
    class [[nodiscard]] Result<void> : public ResultMonad {
    public:
        using value_type = void;
        using error_type = Diagnostic;


        Result(const ok_t) noexcept :
            m_error(nullopt) {
        }

        Result(const error_type& error) :
            m_error(error) {
        }
        Result(error_type&& error) :
            m_error(std::move(error)) {
        }

        constexpr Result(Result&& other) noexcept :
            m_error(other.has_error() ? optional(other.error()) : nullopt) {
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return !m_error.has_value(); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return m_error.has_value(); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        constexpr const error_type& error() const { return *m_error; }


        // Modifiers

        // Needed just in case you want to extend error's message
        constexpr error_type&& extract_error() && { return std::move(*m_error); }


    protected:
        optional<error_type> m_error;
    };

    using Status = Result<void>;
}


// Exports

namespace ers {
    using impl::Result;

    using impl::swap;
    using impl::operator==;

    using impl::ok_t;
    using impl::ok;

    using impl::Status;
}


// Macro

#define ERS_TRY(EXPR) if (auto&& _ers_try = (EXPR); !_ers_try) return std::move(_ers_try).extract_error()

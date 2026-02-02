#pragma once

// std
#include <concepts>
#include <variant>

// ers
#include <erslib/error/error.hpp>

// Forward decl

namespace ers {
    template<typename TError>
    class Unexpected;

    template<typename T, typename TError>
    class Result;
}

// Unexpected

namespace ers {
    template<typename TError = Error>
    class Unexpected {
        using error_type = TError;

    public:
        constexpr explicit Unexpected(const error_type& error)
            requires std::copy_constructible<error_type> :
            _error(error) {
        }
        constexpr explicit Unexpected(error_type&& error)
            requires std::move_constructible<error_type> :
            _error(std::move(error)) {
        }

        template<typename... TArgs>
            requires std::constructible_from<error_type, TArgs...>
        constexpr explicit Unexpected(TArgs&&... args) :
            _error(std::forward<TArgs>(args)...) {
        }

        constexpr Unexpected(const Unexpected&) = default;
        constexpr Unexpected(Unexpected&&) = default;

        constexpr Unexpected& operator=(const Unexpected&) = default;
        constexpr Unexpected& operator=(Unexpected&&) = default;

        constexpr const error_type& error() const & noexcept { return _error; }
        constexpr error_type& error() & noexcept { return _error; }
        constexpr const error_type&& error() const && noexcept { return std::move(_error); }
        constexpr error_type&& error() && noexcept { return std::move(_error); }

        constexpr void swap(Unexpected& other) noexcept {
            std::swap(_error, other._error);
        }

    private:
        error_type _error;
    };

    template<typename TError>
    Unexpected(TError) -> Unexpected<TError>;

    template<typename TError>
    constexpr bool operator==(const Unexpected<TError>& lhs, const Unexpected<TError>& rhs) {
        return lhs.error() == rhs.error();
    }

    template<typename TError>
    constexpr bool operator!=(const Unexpected<TError>& lhs, const Unexpected<TError>& rhs) {
        return !(lhs == rhs);
    }

    template<typename TError>
    constexpr Unexpected<std::decay_t<TError>> make_unexpected(TError&& error) {
        return Unexpected<std::decay_t<TError>>(std::forward<TError>(error));
    }

    template<typename TError, typename... TArgs>
    constexpr Unexpected<TError> make_unexpected(TArgs&&... args) {
        return Unexpected<TError>(std::forward<TArgs>(args)...);
    }

    template<typename... TArgs>
    constexpr auto make_unexpected_from(TArgs&&... args) {
        using TError = std::remove_cvref_t<decltype(TError(std::forward<TArgs>(args)...))>;
        return Unexpected<TError>(std::forward<TArgs>(args)...);
    }
}

// Result

namespace ers {
    template<typename T, typename TError = Error>
    class [[nodiscard]] Result {
        using value_type = T;
        using error_type = TError;

    public:
        constexpr Result()
            requires std::is_default_constructible_v<T> :
            m_variant(T {}) {
        }

        constexpr Result(const value_type& value) :
            m_variant(value) {
        }

        constexpr Result(value_type&& value) noexcept :
            m_variant(std::move(value)) {
        }

        constexpr Result(const Unexpected<error_type>& unexpected) :
            m_variant(std::in_place_index<1>, unexpected.error()) {
        }

        constexpr Result(Unexpected<error_type>&& unexpected) :
            m_variant(std::in_place_index<1>, std::move(unexpected).error()) {
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

        constexpr Result& operator=(const Unexpected<error_type>& unexpected) {
            m_variant = unexpected.error();
            return *this;
        }
        constexpr Result& operator=(Unexpected<error_type>&& unexpected) {
            m_variant = std::move(unexpected).error();
            return *this;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::holds_alternative<value_type>(m_variant); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::holds_alternative<error_type>(m_variant); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        constexpr const value_type& value() & { return std::get<value_type>(m_variant); }
        constexpr const value_type& value() const & { return std::get<value_type>(m_variant); }
        constexpr const value_type& value() && { return std::move(std::get<value_type>(m_variant)); }
        constexpr const value_type& value() const && { return std::move(std::get<value_type>(m_variant)); }

        constexpr const value_type& operator*() & { return value(); }
        constexpr const value_type& operator*() const & { return value(); }
        constexpr const value_type& operator*() && { return std::move(value()); }
        constexpr const value_type& operator*() const && { return std::move(value()); }

        constexpr value_type* operator->() { return value(); }
        constexpr const value_type* operator->() const { return value(); }

        constexpr const error_type& error() & { return std::get<error_type>(m_variant); }
        constexpr const error_type& error() const & { return std::get<error_type>(m_variant); }

    protected:
        std::variant<value_type, error_type> m_variant;
    };

    template<typename T>
    Result(T) -> Result<T>;

    template<typename TError>
    Result(Unexpected<TError>) -> Result<std::monostate, TError>;

    template<typename T, typename TError>
    constexpr void swap(Result<T, TError>& lhs, Result<T, TError>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    template<typename T, typename TError>
    constexpr bool operator==(const Result<T, TError>& lhs, const Result<T, TError>& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;

        if (lhs.has_value())
            return lhs.value() == rhs.value();

        return lhs.error() == rhs.error();
    }

    template<typename T, typename TError>
    constexpr Result<T, TError> make_result(const T& value) {
        return Result<T, TError>(value);
    }

    template<typename T, typename TError>
    constexpr Result<T, TError> make_result(T&& value) {
        return Result<T, TError>(std::move(value));
    }

    template<typename T, typename TError>
    constexpr Result<T, TError> make_result_from_error(const TError& error) {
        return Result<T, TError>(Unexpected<TError>(error));
    }

    template<typename T, typename TError>
    constexpr Result<T, TError> make_result_from_error(TError&& error) {
        return Result<T, TError>(Unexpected<TError>(std::move(error)));
    }
}

// Result<void, ...>

namespace ers {
    struct ok_t {};
    constexpr ok_t ok;

    template<typename TError>
    class [[nodiscard]] Result<void, TError> {
        using value_type = void;
        using error_type = TError;

    public:
        constexpr Result(ok_t) noexcept :
            has_value_(true),
            error_ {} {
        }

        constexpr Result(const Unexpected<error_type>& unexpected) :
            has_value_(false),
            error_(unexpected.error()) {
        }
        constexpr Result(Unexpected<error_type>&& unexpected) :
            has_value_(false),
            error_(std::move(unexpected).error()) {
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }
        [[nodiscard]] constexpr bool has_error() const noexcept { return !has_value_; }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        constexpr const error_type& error() const & { return error_; }
        constexpr error_type& error() & { return error_; }

    private:
        bool has_value_;
        error_type error_;
    };

    using Status = Result<Error>;
}

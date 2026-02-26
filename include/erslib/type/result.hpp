#pragma once

// std
#include <concepts>
#include <variant>

// boost
#include <boost/optional.hpp>

// ers
#include <erslib/type/error.hpp>
#include <erslib/type/ref.hpp>


// Forward decl

namespace ers {
    template<typename E>
    class Unexpected;

    template<typename T, typename E>
    class Result;
}


// Unexpected

namespace ers {
    template<typename E>
    class Unexpected {
        using error_type = E;


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

    template<typename E>
    Unexpected(E) -> Unexpected<E>;

    template<typename E>
    constexpr bool operator==(const Unexpected<E>& lhs, const Unexpected<E>& rhs) {
        return lhs.error() == rhs.error();
    }

    template<typename E>
    constexpr bool operator!=(const Unexpected<E>& lhs, const Unexpected<E>& rhs) {
        return !(lhs == rhs);
    }

    template<typename E>
    constexpr Unexpected<std::decay_t<E>> make_unexpected(E&& error) {
        return Unexpected<std::decay_t<E>>(std::forward<E>(error));
    }

    template<typename E, typename... Args>
    constexpr Unexpected<E> make_unexpected(Args&&... args) {
        return Unexpected<E>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr auto make_unexpected_from(Args&&... args) {
        using E = std::remove_cvref_t<decltype(E(std::forward<Args>(args)...))>;
        return Unexpected<E>(std::forward<Args>(args)...);
    }
}


// Result

namespace ers {
    template<typename T, typename E = Error>
    class [[nodiscard]] Result {
    public:
        // Constructors

        constexpr Result()
            requires std::is_default_constructible_v<T> :
            m_variant(T {}) {
        }

        constexpr Result(const T& value) :
            m_variant(value) {
        }

        constexpr Result(T&& value) noexcept :
            m_variant(std::move(value)) {
        }

        constexpr Result(const Unexpected<E>& unexpected) :
            m_variant(std::in_place_index<1>, unexpected.error()) {
        }

        constexpr Result(Unexpected<E>&& unexpected) :
            m_variant(std::in_place_index<1>, std::move(unexpected).error()) {
        }

        constexpr Result(const Result&) = default;
        constexpr Result(Result&&) = default;

        constexpr Result& operator=(const Result&) = default;
        constexpr Result& operator=(Result&&) = default;

        constexpr Result& operator=(const T& value) {
            m_variant = value;
            return *this;
        }
        constexpr Result& operator=(T&& value) {
            m_variant = std::move(value);
            return *this;
        }

        constexpr Result& operator=(const Unexpected<E>& unexpected) {
            m_variant = unexpected.error();
            return *this;
        }
        constexpr Result& operator=(Unexpected<E>&& unexpected) {
            m_variant = std::move(unexpected).error();
            return *this;
        }


        // Observers

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::holds_alternative<T>(m_variant); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::holds_alternative<E>(m_variant); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }


        // Accessors

        constexpr T& value() & { return std::get<0>(m_variant); }
        constexpr const T& value() const & { return std::get<0>(m_variant); }
        constexpr T&& value() && { return std::move(std::get<0>(m_variant)); }
        constexpr const T& value() const && { return std::move(std::get<0>(m_variant)); }

        constexpr T& operator*() & { return value(); }
        constexpr const T& operator*() const & { return value(); }
        constexpr T&& operator*() && { return std::move(value()); }
        constexpr const T& operator*() const && { return std::move(value()); }

        constexpr T* operator->() { return value(); }
        constexpr const T* operator->() const { return value(); }

        constexpr const E& error() { return std::get<E>(m_variant); }
        constexpr const E& error() const { return std::get<E>(m_variant); }


    protected:
        std::variant<T, E> m_variant;
    };

    template<typename T, typename E>
    class [[nodiscard]] Result<T&, E> {
    public:
        // Constructors

        constexpr Result()
            requires std::is_default_constructible_v<T> :
            m_variant(T {}) {
        }

        constexpr Result(const T& value) :
            m_variant(value) {
        }

        constexpr Result(T&& value) noexcept :
            m_variant(std::move(value)) {
        }

        constexpr Result(const Unexpected<E>& unexpected) :
            m_variant(std::in_place_index<1>, unexpected.error()) {
        }

        constexpr Result(Unexpected<E>&& unexpected) :
            m_variant(std::in_place_index<1>, std::move(unexpected).error()) {
        }

        constexpr Result(const Result&) = default;
        constexpr Result(Result&&) = default;

        constexpr Result& operator=(const Result&) = default;
        constexpr Result& operator=(Result&&) = default;

        constexpr Result& operator=(const T& value) {
            m_variant = value;
            return *this;
        }
        constexpr Result& operator=(T&& value) {
            m_variant = std::move(value);
            return *this;
        }

        constexpr Result& operator=(const Unexpected<E>& unexpected) {
            m_variant = unexpected.error();
            return *this;
        }
        constexpr Result& operator=(Unexpected<E>&& unexpected) {
            m_variant = std::move(unexpected).error();
            return *this;
        }


        // Observers

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::holds_alternative<ref<T>>(m_variant); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::holds_alternative<E>(m_variant); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }


        // Accessors

        constexpr T& value() & { return *std::get<0>(m_variant); }
        constexpr const T& value() const & { return *std::get<0>(m_variant); }
        constexpr T&& value() && { return std::move(*std::get<0>(m_variant)); }
        constexpr const T& value() const && { return std::move(*std::get<0>(m_variant)); }

        constexpr T& operator*() & { return value(); }
        constexpr const T& operator*() const & { return value(); }
        constexpr T&& operator*() && { return std::move(value()); }
        constexpr const T& operator*() const && { return std::move(value()); }

        constexpr T* operator->() { return value(); }
        constexpr const T* operator->() const { return value(); }

        constexpr const E& error() { return std::get<1>(m_variant); }
        constexpr const E& error() const { return std::get<1>(m_variant); }


    protected:
        std::variant<ref<T>, E> m_variant;
    };


    template<typename T>
    Result(T) -> Result<T>;

    template<typename E>
    Result(Unexpected<E>) -> Result<std::monostate, E>;

    template<typename T, typename E>
    constexpr void swap(Result<T, E>& lhs, Result<T, E>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    template<typename T, typename E>
    constexpr bool operator==(const Result<T, E>& lhs, const Result<T, E>& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;

        if (lhs.has_value())
            return lhs.value() == rhs.value();

        return lhs.error() == rhs.error();
    }

    template<typename T, typename E>
    constexpr Result<T, E> make_result(const T& value) {
        return Result<T, E>(value);
    }

    template<typename T, typename E>
    constexpr Result<T, E> make_result(T&& value) {
        return Result<T, E>(std::move(value));
    }

    template<typename T, typename E>
    constexpr Result<T, E> make_result_from_error(const E& error) {
        return Result<T, E>(Unexpected<E>(error));
    }

    template<typename T, typename E>
    constexpr Result<T, E> make_result_from_error(E&& error) {
        return Result<T, E>(Unexpected<E>(std::move(error)));
    }
}


// Result<void, ...>

namespace ers {
    struct ok_t {};

    constexpr ok_t ok;

    template<typename E>
    class [[nodiscard]] Result<void, E> {
        using value_type = void;
        using error_type = E;


    public:
        constexpr Result(const ok_t) noexcept :
            m_error(boost::none) {
        }

        constexpr Result(const Unexpected<error_type>& unexpected) :
            m_error(unexpected.error()) {
        }
        constexpr Result(Unexpected<error_type>&& unexpected) :
            m_error(std::move(unexpected.error())) {
        }

        template<typename T>
        constexpr Result(Result<T, E>&& other) :
            m_error(other.has_error() ? other.error() : boost::none) {
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return !m_error.has_value(); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return m_error.has_value(); }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

        constexpr error_type& error() { return *m_error; }
        constexpr const error_type& error() const { return *m_error; }


    protected:
        boost::optional<error_type> m_error;
    };

    using Status = Result<void>;
}

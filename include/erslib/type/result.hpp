#pragma once

// std
#include <variant>

// ers
#include <erslib/type/error.hpp>
#include <erslib/type/optional.hpp>
#include <erslib/type/ref.hpp>


// Forward decl

namespace ers {
    template<typename T>
    class Result;
}


// Result<T>

namespace ers {
    template<typename T>
    class [[nodiscard]] Result {
    public:
        using value_type = T;
        using error_type = Error;


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

namespace ers {
    template<typename T>
    class [[nodiscard]] Result<T&> {
    public:
        using value_type = T;
        using error_type = Error;


        // Constructors

        constexpr Result()
            requires std::is_default_constructible_v<value_type> :
            m_variant(value_type {}) {
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

namespace ers {
    template<typename T>
    Result(T) -> Result<T>;

    template<typename T>
    constexpr void swap(Result<T>& lhs, Result<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    template<typename T, typename E>
    constexpr bool operator==(const Result<T>& lhs, const Result<T>& rhs) {
        if (lhs.has_value() != rhs.has_value())
            return false;

        if (lhs.has_value())
            return lhs.value() == rhs.value();

        return lhs.error() == rhs.error();
    }
}


// Result<void, ...>

namespace ers {
    struct ok_t {};

    constexpr ok_t ok;

    template<>
    class [[nodiscard]] Result<void> {
    public:
        using value_type = void;
        using error_type = Error;


        Result(const ok_t) noexcept :
            m_error(nullopt) {
        }

        Result(const error_type& error) :
            m_error(error) {
        }
        Result(error_type&& error) :
            m_error(std::move(error)) {
        }

        template<typename T>
        constexpr Result(Result&& other) noexcept :
            m_error(other.has_error() ? optional<error_type>(other.error()) : nullopt) {
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

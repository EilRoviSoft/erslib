#pragma once

// std
#include <string>
#include <variant>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/none.hpp>

// aengine
#include <erslib/aengine/fwd.hpp>


namespace aengine::internal {
    template<typename T>
    struct value_bounds_t {
        T min;
        T max;
    };

    template<typename T>
    using allowed_values_t = OrderedSet<T>;


    template<typename T>
    using check_states_t = std::variant<
        ers::none_t,
        value_bounds_t<T>,
        allowed_values_t<T>
    >;


    template<typename T>
    struct check_state_visitor {
        bool operator()(const T&, ers::none_t) const { return true; }
        
        bool operator()(const T& value, const value_bounds_t<T>& value_bounds) const {
            return value >= value_bounds.min && value <= value_bounds.max;
        }

        bool operator()(const T& value, const allowed_values_t<T>& allowed_values) const {
            return allowed_values.contains(value);
        }
    };
}


namespace aengine::internal {
    template<typename T, typename ViewT = T>
    class BaseSetting {
    public:
        using Type = T;
        using ViewType = ViewT;

        // Modifiers

        void set(ViewT value) { m_value = value; }


        // Accessors

        [[nodiscard]]
        ViewT value() const { return m_value; }

        [[nodiscard]]
        ViewT default_value() const { return m_default_value; }


    protected:
        T m_value;
        T m_default_value;


        // Member functions

        explicit BaseSetting(ViewT default_value) :
            m_default_value(default_value) {
            m_value = m_default_value;
        }
    };
}


namespace aengine {
    class BoolSetting : internal::BaseSetting<bool> {
    public:
        BoolSetting(ViewType default_value, ViewType forced_value);

    protected:
        Type m_forced_value;
    };

    class IntSetting : internal::BaseSetting<i64> {
    public:

    protected:
        internal::check_states_t<Type> m_check_states;
    };

    class DoubleSetting : internal::BaseSetting<f64> {
    public:

    protected:
        internal::check_states_t<Type> m_check_states;
    };

    class StringSetting : internal::BaseSetting<std::string, std::string_view> {
    public:

    protected:
        bool m_allow_blank, m_auto_trim;
        internal::allowed_values_t<Type> m_allowed_values;
    };
}

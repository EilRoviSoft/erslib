#pragma once

// std
#include <utility>

// sol
#include <sol/table.hpp>

// ers
#include <erslib/adaptor/transparent/base.hpp>

// aengine
#include <aengine/fwd.hpp>


// field_property_t

namespace aengine {
    // TODO: add more properties
    struct field_property_t {
        enum property_type {
            None = 0,
            Required
        };


        property_type type;
        union {
            bool is_required;
        };


        static field_property_t required(bool value);
    };
}


template<typename Policy>
struct ers::THashBase<aengine::field_property_t, Policy> {
    using as_integral_type = std::underlying_type_t<aengine::field_property_t::property_type>;


    constexpr size_t operator()(aengine::field_property_t what, size_t seed = 0) const noexcept {
        return ers::THashBase<as_integral_type, Policy> {}(std::to_underlying(what.type), seed);
    }
};


namespace aengine::internal {
    struct field_property_equal {
        using is_transparent = void;


        constexpr bool operator()(const field_property_t& lhs, const field_property_t& rhs) const noexcept {
            return lhs.type == rhs.type;
        }

        constexpr bool operator()(const field_property_t& lhs, size_t rhs) const noexcept {
            return static_cast<size_t>(std::to_underlying(lhs.type)) == rhs;
        }

        constexpr bool operator()(size_t lhs, const field_property_t& rhs) const noexcept {
            return lhs == static_cast<size_t>(std::to_underlying(rhs.type));
        }
    };


    using FieldPropertySet = HashSet<
        field_property_t,
        ers::member_hash_adaptor<ers::hashing::direct_policy, &field_property_t::type, size_t>,
        ers::member_equal_adaptor<&field_property_t::type>
    >;
}


// Layout

namespace aengine {
    // In the best world you should allocate 'Layout' once and never change it again.
    // Otherwise, this could lead to UB.
    class Layout {
        struct field_t {
            std::string type;
            internal::FieldPropertySet properties;
        };


    public:
        // Modifiers

        void add_field(std::string_view name, std::string_view type, internal::FieldPropertySet properties);
        void add_fields(std::initializer_list<std::tuple<std::string_view, std::string_view, internal::FieldPropertySet>> info);

        void inherit(const Layout& layout);


        // Observers

        template<typename K>
        const field_t& field(const K& k) const {
            return m_fields[k];
        }

        template<typename K>
        const field_t& operator[](const K& k) {
            return field(k);
        }


        std::string_view name() const noexcept { return m_name; }


    protected:
        std::string m_name;
        StringMap<field_t> m_fields;
    };


    using LayoutDictionary = HashSet<
        Layout,
        ers::member_string_hash_adaptor<ers::hashing::rapid_policy, &Layout::name>,
        ers::member_equal_adaptor<&Layout::name>
    >;
}


// Prototype

namespace aengine {
    class Prototype {
    public:


    protected:
        std::string m_name;
        std::string m_type;
        StringMap<Object> m_components;
    };
}

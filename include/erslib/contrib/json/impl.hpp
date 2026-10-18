#pragma once

// std
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// ers
#include <erslib/core/adaptor.hpp>
#include <erslib/core/enum.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/optional.hpp>

// export
#include <erslib/export.hpp>

#ifdef _HAS_BOOST_UNORDERED

#include <boost/unordered/unordered_flat_map.hpp>

#endif


// Traits ----------------------------------------------------------------------

namespace utl {
    template<class S>
    struct string_adaptor;

    template<>
    struct string_adaptor<std::string> {
        [[nodiscard]]
        static std::string from_utf8(std::string&& value) noexcept {
            return std::move(value);
        }

        [[nodiscard]]
        static std::string_view to_utf8(const std::string& value) noexcept {
            return value;
        }
    };
}


namespace utl::impl {
    template<class T>
    using flat_object =
#ifdef _HAS_BOOST_UNORDERED
        boost::unordered_flat_map<
#else
        std::unordered_map<
#endif
            std::string, T,
            ers::adaptor::string_hash<ers::RapidHash>,
            ers::adaptor::equal<std::string>
        >;


    struct default_traits {
        template<class Node>
        using object_type = flat_object<Node>;

        template<class Node>
        using array_type = std::vector<Node>;

        using string_type = std::string;
        using integral_type = i64;
        using floating_type = f64;
    };


    template<class Traits>
    concept JsonTraits = requires {
            typename Traits::string_type;
            typename Traits::template object_type<int>;
            typename Traits::template array_type<int>;
        }
        && std::signed_integral<typename Traits::integral_type>
        && std::floating_point<typename Traits::floating_type>
        && requires(std::string utf8, const typename Traits::string_type& value) {
            { string_adaptor<typename Traits::string_type>::from_utf8(std::move(utf8)) } -> std::same_as<typename Traits::string_type>;
            { string_adaptor<typename Traits::string_type>::to_utf8(value) } -> std::convertible_to<std::string_view>;
        }
        && requires(std::string utf8) {
            { string_adaptor<typename Traits::template object_type<int>::key_type>::from_utf8(std::move(utf8)) }
            -> std::same_as<typename Traits::template object_type<int>::key_type>;
        };
}


// Node ------------------------------------------------------------------------

namespace utl::impl {
    enum class EFormat : u8 {
        Pretty,
        Minimized,
    };

    enum class ENodeType : u8 {
        None = 0,
        Object = 1,
        Array = 2,
        String = 3,
        Integral = 4,
        Floating = 5,
        Bool = 6,
    };


    struct null_type_impl {
        [[nodiscard]] bool operator==(const null_type_impl&) const noexcept = default;
    };


    template<JsonTraits Traits>
    class TNode {
    public:
        // -- Public Traits --
        // -------------------

        using traits_type = Traits;

        using object_type = typename Traits::template object_type<TNode>;
        using array_type = typename Traits::template array_type<TNode>;
        using string_type = typename Traits::string_type;
        using integral_type = typename Traits::integral_type;
        using floating_type = typename Traits::floating_type;
        using bool_type = bool;
        using null_type = null_type_impl;

        using variant_type = std::variant<
            null_type,
            object_type,
            array_type,
            string_type,
            integral_type,
            floating_type,
            bool_type
        >;


        // -- Constructors --
        // ------------------

        TNode() = default;
        TNode(const TNode&) = default;
        TNode(TNode&&) noexcept = default;
        ~TNode() = default;


        TNode(null_type) noexcept {}
        TNode(std::nullptr_t) noexcept {}

        TNode(bool_type value) noexcept :
            _data(std::in_place_type<bool_type>, value) {
        }

        template<std::integral T>
            requires (!std::same_as<T, bool>)
        TNode(T value) :
            _data(std::in_place_type<integral_type>, checked_integral(value)) {
        }

        template<std::floating_point T>
        TNode(T value) noexcept :
            _data(std::in_place_type<floating_type>, static_cast<floating_type>(value)) {
        }

        TNode(const char* value) :
            TNode(std::string_view(value)) {
        }

        TNode(std::string_view value) :
            _data(std::in_place_type<string_type>, string_adaptor<string_type>::from_utf8(std::string(value))) {
        }

        TNode(const string_type& value)
            requires (!std::same_as<string_type, std::string_view>) :
            _data(std::in_place_type<string_type>, value) {
        }
        TNode(string_type&& value) :
            _data(std::in_place_type<string_type>, std::move(value)) {
        }

        TNode(const object_type& value) :
            _data(std::in_place_type<object_type>, value) {
        }
        TNode(object_type&& value) :
            _data(std::in_place_type<object_type>, std::move(value)) {
        }

        TNode(const array_type& value) :
            _data(std::in_place_type<array_type>, value) {
        }
        TNode(array_type&& value) :
            _data(std::in_place_type<array_type>, std::move(value)) {
        }


        // -- Assignment --
        // ----------------

        TNode& operator=(const TNode& other) {
            auto data = other._data;
            _data.swap(data);
            return *this;
        }

        TNode& operator=(TNode&& other) noexcept {
            auto data = std::move(other._data);
            _data.swap(data);
            return *this;
        }


        // -- Type information --
        // ----------------------

        [[nodiscard]] ENodeType type() const noexcept {
            return static_cast<ENodeType>(_data.index());
        }


        template<class T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(_data);
        }

        [[nodiscard]] bool is_object() const noexcept { return is<object_type>(); }
        [[nodiscard]] bool is_array() const noexcept { return is<array_type>(); }
        [[nodiscard]] bool is_string() const noexcept { return is<string_type>(); }
        [[nodiscard]] bool is_integral() const noexcept { return is<integral_type>(); }
        [[nodiscard]] bool is_floating() const noexcept { return is<floating_type>(); }
        [[nodiscard]] bool is_bool() const noexcept { return is<bool_type>(); }
        [[nodiscard]] bool is_null() const noexcept { return is<null_type>(); }


        // -- Getters --
        // -------------

        template<class T>
        [[nodiscard]] T* get_if() noexcept {
            return std::get_if<T>(&_data);
        }

        template<class T>
        [[nodiscard]] const T* get_if() const noexcept {
            return std::get_if<T>(&_data);
        }

        template<class T>
        [[nodiscard]] T& as() {
            return std::get<T>(_data);
        }

        template<class T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(_data);
        }

        [[nodiscard]] object_type& as_object() { return as<object_type>(); }
        [[nodiscard]] array_type& as_array() { return as<array_type>(); }
        [[nodiscard]] string_type& as_string() { return as<string_type>(); }
        [[nodiscard]] integral_type& as_integral() { return as<integral_type>(); }
        [[nodiscard]] floating_type& as_floating() { return as<floating_type>(); }
        [[nodiscard]] bool_type& as_bool() { return as<bool_type>(); }
        [[nodiscard]] null_type& as_null() { return as<null_type>(); }

        [[nodiscard]] const object_type& as_object() const { return as<object_type>(); }
        [[nodiscard]] const array_type& as_array() const { return as<array_type>(); }
        [[nodiscard]] const string_type& as_string() const { return as<string_type>(); }
        [[nodiscard]] const integral_type& as_integral() const { return as<integral_type>(); }
        [[nodiscard]] const floating_type& as_floating() const { return as<floating_type>(); }
        [[nodiscard]] const bool_type& as_bool() const { return as<bool_type>(); }
        [[nodiscard]] const null_type& as_null() const { return as<null_type>(); }


        // -- Object access --
        // -------------------

        TNode& operator[](std::string_view key);
        [[nodiscard]] const TNode& operator[](std::string_view key) const;

        [[nodiscard]] TNode& at(std::string_view key);
        [[nodiscard]] const TNode& at(std::string_view key) const;

        [[nodiscard]] ers::optional<TNode&> try_at(std::string_view key);
        [[nodiscard]] ers::optional<const TNode&> try_at(std::string_view key) const;

        [[nodiscard]] bool contains(std::string_view key) const;


        // -- Array access --
        // ------------------

        [[nodiscard]] TNode& operator[](std::size_t index);
        [[nodiscard]] const TNode& operator[](std::size_t index) const;

        [[nodiscard]] TNode& at(std::size_t index);
        [[nodiscard]] const TNode& at(std::size_t index) const;

        [[nodiscard]] ers::optional<TNode&> try_at(std::size_t index);
        [[nodiscard]] ers::optional<const TNode&> try_at(std::size_t index) const;

        void push_back(const TNode& value);
        void push_back(TNode&& value);

        template<class... Args>
            requires std::constructible_from<TNode, Args...>
        TNode& emplace_back(Args&&... args);


    private:
        variant_type _data {};


        template<std::integral T>
        [[nodiscard]] static integral_type checked_integral(T value) {
            if (!std::in_range<integral_type>(value)) {
                throw ers::make_out_of_range_error("Integer {} doesn't fit into the JSON integral type",
                    value);
            }

            return static_cast<integral_type>(value);
        }


        [[nodiscard]] TNode* find_child(std::string_view key);
        [[nodiscard]] const TNode* find_child(std::string_view key) const;

        [[nodiscard]] array_type& array_or_throw(std::size_t index);
        [[nodiscard]] const array_type& array_or_throw(std::size_t index) const;


        // -- Asserts --
        // -------------

        static_assert(std::variant_size_v<variant_type> == 7);

        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::None), variant_type>, null_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::Object), variant_type>, object_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::Array), variant_type>, array_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::String), variant_type>, string_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::Integral), variant_type>, integral_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::Floating), variant_type>, floating_type>);
        static_assert(std::same_as<std::variant_alternative_t<std::to_underlying(ENodeType::Bool), variant_type>, bool_type>);
    };
}


// Exports ---------------------------------------------------------------------

namespace utl::impl {
    extern template class ERSLIB_EXPORT TNode<default_traits>;

    using Node = TNode<default_traits>;
}


// Implementation --------------------------------------------------------------

#include <erslib/contrib/json/impl.inl>

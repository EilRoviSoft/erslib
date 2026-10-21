#pragma once

// std
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// boost
#include <boost/container/map.hpp>

// ers
#include <erslib/contrib/json/pointer.hpp>
#include <erslib/core/adaptor.hpp>
#include <erslib/core/enum.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/filesystem.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/maybe.hpp>

// export
#include <erslib/export.hpp>


#ifdef _HAS_BOOST_UNORDERED

#include <boost/unordered/unordered_flat_map.hpp>

#endif


// Traits ----------------------------------------------------------------------

namespace utl {
    template<typename S>
    struct string_adaptor;

    template<>
    struct string_adaptor<std::string> {
        using string_type = std::string;


        [[nodiscard]]
        static string_type from_utf8(std::string&& value) noexcept {
            return std::move(value);
        }

        [[nodiscard]]
        static std::string_view to_utf8(const string_type& value) noexcept {
            return value;
        }
    };
}


namespace utl::impl::json_detail {
    struct no_extension {};
}


namespace utl::impl {
    template<typename T>
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
        static constexpr bool assign_by_value = false;


        template<typename Node>
        using object_type = flat_object<Node>;

        template<typename Node>
        using array_type = std::vector<Node>;

        template<typename Node>
        using extension_type = json_detail::no_extension;

        using string_type = std::string;
        using integral_type = i64;
        using floating_type = f64;
    };


    template<typename T>
    using ordered_object = boost::container::map<std::string, T, std::less<>>;

    struct ordered_traits : default_traits {
        template<typename Node>
        using object_type = ordered_object<Node>;
    };


    template<typename Traits>
    concept JsonTraits = requires {
            typename Traits::string_type;
            typename Traits::template object_type<int>;
            typename Traits::template array_type<int>;
            typename Traits::template extension_type<int>;
        }
        && std::signed_integral<typename Traits::integral_type>
        && std::floating_point<typename Traits::floating_type>
        && std::same_as<std::remove_cv_t<decltype(Traits::assign_by_value)>, bool>
        && requires(std::string utf8, const typename Traits::string_type& value) {
            { string_adaptor<typename Traits::string_type>::from_utf8(std::move(utf8)) } -> std::same_as<typename Traits::string_type>;
            { string_adaptor<typename Traits::string_type>::to_utf8(value) } -> std::convertible_to<std::string_view>;
        }
        && requires(std::string utf8) {
            { string_adaptor<typename Traits::template object_type<int>::key_type>::from_utf8(std::move(utf8)) }
            -> std::same_as<typename Traits::template object_type<int>::key_type>;
        };
}


// Details ---------------------------------------------------------------------

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


    [[nodiscard]]
    constexpr std::string_view type_label(ENodeType value) noexcept {
        return ers::enum_utils::to_string(value);
    }


    struct null_type_impl {
        [[nodiscard]] bool operator==(const null_type_impl&) const noexcept = default;
    };


    template<JsonTraits Traits>
    class TNode;
}


// Conversion ------------------------------------------------------------------

namespace utl::impl::json_detail {
    enum class ESourceKind : u8 {
        None,
        Node,
        OtherNode,
        Null,
        Boolean,
        String,
        Integral,
        Floating,
        Optional,
        Object,
        Array,
    };
}


namespace utl::impl::json_detail {
    template<typename T>
    constexpr bool is_node_v = false;

    template<typename Traits>
    constexpr bool is_node_v<TNode<Traits>> = true;
}


namespace utl::impl {
    template<class T>
    concept BoolLike = std::same_as<T, bool> || std::same_as<T, std::vector<bool>::reference>;


    template<typename T>
    concept Utf8Viewable = std::convertible_to<const T&, std::string_view>;

    template<typename T>
    concept AdaptedString = requires(const T & value) {
        { string_adaptor<T>::to_utf8(value) } -> std::convertible_to<std::string_view>;
    };

    template<typename T>
    concept StringLike = Utf8Viewable<T> || AdaptedString<T>;


    template<typename T>
    concept OptionalLike = requires(T& value) {
        { value.has_value() } -> std::convertible_to<bool>;
        *value;
        value.reset();
    };


    template<typename T>
    concept MapLike = std::ranges::input_range<T> && requires {
        typename T::key_type;
        typename T::mapped_type;
    };
}


namespace utl::impl::json_detail {
    template<typename T, typename Node>
    consteval ESourceKind classify() {
        using U = std::remove_cvref_t<T>;

        if constexpr (std::same_as<U, Node>) {
            return ESourceKind::Node;
        } else if constexpr (is_node_v<U>) {
            return ESourceKind::OtherNode;
        } else if constexpr (std::same_as<U, null_type_impl> || std::same_as<U, std::nullptr_t>) {
            return ESourceKind::Null;
        } else if constexpr (BoolLike<U>) {
            return ESourceKind::Boolean;
        } else if constexpr (StringLike<U>) {
            return ESourceKind::String;
        } else if constexpr (std::integral<U>) {
            return ESourceKind::Integral;
        } else if constexpr (std::floating_point<U>) {
            return ESourceKind::Floating;
        } else if constexpr (OptionalLike<U>) {
            if constexpr (classify<decltype(*std::declval<const U&>()), Node>() != ESourceKind::None)
                return ESourceKind::Optional;
            else
                return ESourceKind::None;
        } else if constexpr (MapLike<U>) {
            if constexpr (StringLike<typename U::key_type>) {
                if constexpr (classify<typename U::mapped_type, Node>() != ESourceKind::None)
                    return ESourceKind::Object;
                else
                    return ESourceKind::None;
            } else
                return ESourceKind::None;
        } else if constexpr (std::ranges::input_range<U>) {
            using item_type = std::remove_cvref_t<std::ranges::range_reference_t<U>>;

            if constexpr (std::same_as<item_type, U>) {
                return ESourceKind::None;
            } else {
                if constexpr (classify<std::ranges::range_reference_t<U>, Node>() != ESourceKind::None)
                    return ESourceKind::Array;
                else
                    return ESourceKind::None;
            }
        } else
            return ESourceKind::None;
    }
}


namespace utl::impl::json_detail {
    template<typename T, typename Node>
    concept ConvertibleSource = classify<T, Node>() != ESourceKind::None
        && classify<T, Node>() != ESourceKind::Node;


    template<typename String, typename Source>
    [[nodiscard]]
    String to_string_type(Source&& value) {
        using source_t = std::remove_cvref_t<Source>;

        if constexpr (std::same_as<source_t, String>) {
            return std::forward<Source>(value);
        } else if constexpr (Utf8Viewable<source_t>) {
            auto view = static_cast<std::string_view>(value);
            return string_adaptor<String>::from_utf8(static_cast<std::string>(view));
        } else {
            auto&& utf8 = string_adaptor<source_t>::to_utf8(value);

            if constexpr (std::same_as<std::remove_cvref_t<decltype(utf8)>, std::string>) {
                return string_adaptor<String>::from_utf8(static_cast<std::string>(std::move(utf8)));
            } else {
                auto view = static_cast<std::string_view>(utf8);
                return string_adaptor<String>::from_utf8(static_cast<std::string>(view));
            }
        }
    }
}


namespace utl::impl::json_detail {
    template<typename U, typename Node>
    consteval auto nearest_type() {
        constexpr auto kind = classify<U, Node>();

        if constexpr (kind == ESourceKind::Node)
            return std::type_identity<Node> {};
        else if constexpr (kind == ESourceKind::Boolean)
            return std::type_identity<typename Node::bool_type> {};
        else if constexpr (kind == ESourceKind::String)
            return std::type_identity<typename Node::string_type> {};
        else if constexpr (kind == ESourceKind::Integral)
            return std::type_identity<typename Node::integral_type> {};
        else if constexpr (kind == ESourceKind::Floating)
            return std::type_identity<typename Node::floating_type> {};
        else if constexpr (kind == ESourceKind::Object)
            return std::type_identity<typename Node::object_type> {};
        else if constexpr (kind == ESourceKind::Array)
            return std::type_identity<typename Node::array_type> {};
        else
            return std::type_identity<void> {};
    }

    // the JSON type a fallback of type 'U' is read as
    template<typename U, typename Node>
    using nearest_t = typename decltype(nearest_type<U, Node>())::type;

    template<typename T, typename U, typename Node>
    using value_or_t = std::conditional_t<std::is_void_v<T>, nearest_t<U, Node>, T>;


    template<typename V, typename Node>
    concept NodeValue = std::same_as<V, Node>
        || std::same_as<V, typename Node::bool_type>
        || (std::integral<V> && !std::same_as<V, bool>)
        || std::floating_point<V>
        || std::same_as<V, typename Node::string_type>
        || std::same_as<V, typename Node::object_type>
        || std::same_as<V, typename Node::array_type>;

    // whether a value stored as 'Stored' can be read as 'V'
    template<typename V, typename Stored, typename Node>
    concept ReadableAs = std::same_as<V, Stored>
        || (std::integral<V> && !std::same_as<V, bool> && std::same_as<Stored, typename Node::integral_type>)
        || (std::floating_point<V> && (std::same_as<Stored, typename Node::integral_type> || std::same_as<Stored, typename Node::floating_type>));

    template<typename T, typename U, typename Node>
    concept FallbackFor = !std::is_void_v<nearest_t<U, Node>>
        && (std::is_void_v<T> || (NodeValue<T, Node> && ReadableAs<T, nearest_t<U, Node>, Node>));
}


// Implementation --------------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    class TNode : public Traits::template extension_type<TNode<Traits>> {
        template<JsonTraits>
        friend class TNode;


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

        template<class T>
            requires json_detail::ConvertibleSource<T, TNode>
        TNode(T&& value);

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

        TNode& operator=(TNode other) noexcept
            requires (Traits::assign_by_value) {
            _data.swap(other._data);
            return *this;
        }


        TNode& operator=(const TNode& other)
            requires (!Traits::assign_by_value) {
            auto data = other._data;
            _data.swap(data);
            return *this;
        }

        TNode& operator=(TNode&& other) noexcept
            requires (!Traits::assign_by_value) {
            auto data = std::move(other._data);
            _data.swap(data);
            return *this;
        }


        // -- Type information --
        // ----------------------

        [[nodiscard]] ENodeType type() const noexcept {
            return static_cast<ENodeType>(_data.index());
        }


        template<typename T>
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

        template<typename T>
        [[nodiscard]] T* get_if() noexcept {
            return std::get_if<T>(&_data);
        }

        template<typename T>
        [[nodiscard]] const T* get_if() const noexcept {
            return std::get_if<T>(&_data);
        }

        template<typename T>
        [[nodiscard]] T& as() {
            return std::get<T>(_data);
        }

        template<typename T>
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

        [[nodiscard]] ers::maybe<TNode&> try_at(std::string_view key);
        [[nodiscard]] ers::maybe<const TNode&> try_at(std::string_view key) const;

        [[nodiscard]] bool contains(std::string_view key) const;


        template<typename T = void, typename U>
            requires json_detail::FallbackFor<T, U, TNode>
        [[nodiscard]] json_detail::value_or_t<T, U, TNode> value_or(std::string_view key, U&& fallback) const;


        // -- Array access --
        // ------------------

        [[nodiscard]] TNode& operator[](std::size_t index);
        [[nodiscard]] const TNode& operator[](std::size_t index) const;

        [[nodiscard]] TNode& at(std::size_t index);
        [[nodiscard]] const TNode& at(std::size_t index) const;

        [[nodiscard]] ers::maybe<TNode&> try_at(std::size_t index);
        [[nodiscard]] ers::maybe<const TNode&> try_at(std::size_t index) const;

        void push_back(const TNode& value);
        void push_back(TNode&& value);

        template<typename... Args>
            requires std::constructible_from<TNode, Args...>
        TNode& emplace_back(Args&&... args);


        // -- Pointer access --
        // --------------------

        TNode& operator[](const JsonPointer& pointer);
        [[nodiscard]] const TNode& operator[](const JsonPointer& pointer) const;

        [[nodiscard]] TNode& at(const JsonPointer& pointer);
        [[nodiscard]] const TNode& at(const JsonPointer& pointer) const;

        [[nodiscard]] ers::maybe<TNode&> try_at(const JsonPointer& pointer);
        [[nodiscard]] ers::maybe<const TNode&> try_at(const JsonPointer& pointer) const;

        [[nodiscard]] bool contains(const JsonPointer& pointer) const;


        // -- Serialization --
        // -------------------

        [[nodiscard]]
        std::string to_string(EFormat format = EFormat::Pretty) const;
        
        void to_file(const fs::path& path, EFormat format = EFormat::Pretty) const;


    private:
        variant_type _data {};


        template<std::integral T>
        [[nodiscard]] static integral_type checked_integral(T value);

        template<typename V>
        [[nodiscard]] static V read_value(const TNode& node, std::string_view key);


        [[nodiscard]] TNode* find_child(std::string_view key);
        [[nodiscard]] const TNode* find_child(std::string_view key) const;

        [[nodiscard]] array_type& array_or_throw(std::size_t index);
        [[nodiscard]] const array_type& array_or_throw(std::size_t index) const;

        [[nodiscard]] const TNode* find_pointer(const JsonPointer& pointer) const;


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
    extern template class ERSLIB_EXPORT TNode<ordered_traits>;

    using Json = TNode<default_traits>;
    using OrderedJson = TNode<ordered_traits>;

    using Object = Json::object_type;
    using Array = Json::array_type;
    using String = Json::string_type;
    using Integral = Json::integral_type;
    using Floating = Json::floating_type;
    using Bool = Json::bool_type;
    using Null = Json::null_type;
}


// Storage types ---------------------------------------------------------------

namespace utl::impl::json_detail {
    template<class T, class Node>
    consteval auto storage_of() {
        constexpr auto kind = classify<T, Node>();

        if constexpr (kind == ESourceKind::Null)
            return std::type_identity<typename Node::null_type> {};
        else if constexpr (kind == ESourceKind::Boolean)
            return std::type_identity<typename Node::bool_type> {};
        else if constexpr (kind == ESourceKind::String)
            return std::type_identity<typename Node::string_type> {};
        else if constexpr (kind == ESourceKind::Integral)
            return std::type_identity<typename Node::integral_type> {};
        else if constexpr (kind == ESourceKind::Floating)
            return std::type_identity<typename Node::floating_type> {};
        else if constexpr (kind == ESourceKind::Object)
            return std::type_identity<typename Node::object_type> {};
        else if constexpr (kind == ESourceKind::Array)
            return std::type_identity<typename Node::array_type> {};
        else
            static_assert(false, "T has no single JSON storage type");
    }
}

namespace utl::impl {
    template<class T, class Node = impl::Json>
    using json_storage_t = typename decltype(json_detail::storage_of<T, Node>())::type;

    template<class T>
    struct json_conversion {
        using original_type = json_storage_t<T>;
    };


    namespace literals {
        [[nodiscard]]
        ERSLIB_EXPORT Json operator""_json(const char* chars, std::size_t size);
    }
}


// Implementation --------------------------------------------------------------

#include <erslib/contrib/json/impl.inl>
#include <erslib/contrib/json/io.inl>

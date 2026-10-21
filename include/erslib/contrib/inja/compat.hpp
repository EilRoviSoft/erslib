#pragma once

// std
#include <algorithm>
#include <compare>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// json
#include <erslib/contrib/json.hpp>


// Iterator --------------------------------------------------------------------

namespace utl::impl {
    template<class Node, bool Const>
    class CompatIterator {
        template<class, bool>
        friend class CompatIterator;

        using node_pointer = std::conditional_t<Const, const Node*, Node*>;

        using object_iterator = std::conditional_t<Const,
            typename Node::object_type::const_iterator,
            typename Node::object_type::iterator>;

        using array_iterator = std::conditional_t<Const,
            typename Node::array_type::const_iterator,
            typename Node::array_type::iterator>;

        using position_type = std::variant<std::ptrdiff_t, object_iterator, array_iterator>;


    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Node;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<Const, const Node&, Node&>;
        using pointer = node_pointer;


        CompatIterator() = default;

        operator CompatIterator<Node, true>() const
            requires (!Const) {
            return std::visit([this]<typename Position>(const Position& position) {
                return CompatIterator<Node, true>(_node, position);
            }, _position);
        }


        [[nodiscard]]
        static CompatIterator begin_of(reference node) {
            if (auto* object = node.template get_if<typename Node::object_type>())
                return { &node, object->begin() };
            if (auto* array = node.template get_if<typename Node::array_type>())
                return { &node, array->begin() };
            return { &node, std::ptrdiff_t { node.is_null() ? 1 : 0 } };
        }

        [[nodiscard]]
        static CompatIterator end_of(reference node) {
            if (auto* object = node.template get_if<typename Node::object_type>())
                return { &node, object->end() };
            if (auto* array = node.template get_if<typename Node::array_type>())
                return { &node, array->end() };
            return { &node, std::ptrdiff_t { 1 } };
        }

        [[nodiscard]]
        static CompatIterator from(reference node, object_iterator it) {
            return { &node, it };
        }


        // -- Access --
        // ------------

        [[nodiscard]] reference operator*() const {
            if (const auto* it = std::get_if<object_iterator>(&_position))
                return (*it)->second;
            
            if (const auto* it = std::get_if<array_iterator>(&_position))
                return **it;

            if (std::get<std::ptrdiff_t>(_position) != 0) {
                throw ers::make_out_of_range_error("Can't dereference the end of a JSON {}",
                    type_label(_node->type()));
            }

            return *_node;
        }

        [[nodiscard]] pointer operator->() const {
            return &**this;
        }

        [[nodiscard]] reference operator[](difference_type offset) const {
            return *(*this + offset);
        }


        [[nodiscard]] const auto& key() const {
            if (const auto* it = std::get_if<object_iterator>(&_position))
                return (*it)->first;
            
            throw ers::make_conversion_error("Can't take a key of a JSON {} iterator",
                type_label(_node->type()));
        }

        [[nodiscard]] reference value() const {
            return **this;
        }


        // -- Movement --
        // --------------

        CompatIterator& operator+=(difference_type offset) {
            std::visit([this, offset]<typename Position>(Position& position) {
                if constexpr (std::same_as<Position, object_iterator>)
                    throw ers::make_conversion_error("Can't use offsets with JSON object iterators");
                else
                    position += offset;
            }, _position);

            return *this;
        }

        CompatIterator& operator-=(difference_type offset) {
            return *this += -offset;
        }

        CompatIterator& operator++() {
            std::visit([](auto& position) { ++position; }, _position);
            return *this;
        }

        CompatIterator& operator--() {
            std::visit([](auto& position) { --position; }, _position);
            return *this;
        }

        CompatIterator operator++(int) {
            auto copy = *this;
            ++*this;
            return copy;
        }

        CompatIterator operator--(int) {
            auto copy = *this;
            --*this;
            return copy;
        }


        [[nodiscard]]
        friend CompatIterator operator+(CompatIterator it, difference_type offset) {
            return it += offset;
        }

        [[nodiscard]]
        friend CompatIterator operator+(difference_type offset, CompatIterator it) {
            return it += offset;
        }

        [[nodiscard]]
        friend CompatIterator operator-(CompatIterator it, difference_type offset) {
            return it -= offset;
        }

        [[nodiscard]]
        friend difference_type operator-(const CompatIterator& lhs, const CompatIterator& rhs) {
            return std::visit([&]<typename Position>(const Position& position) -> difference_type {
                if constexpr (std::same_as<Position, object_iterator>)
                    throw ers::make_conversion_error("Can't use offsets with JSON object iterators");
                else
                    return position - std::get<Position>(rhs._position);
            }, lhs._position);
        }


        // -- Comparison --
        // ----------------

        [[nodiscard]]
        friend bool operator==(const CompatIterator& lhs, const CompatIterator& rhs) {
            return lhs._position == rhs._position;
        }

        [[nodiscard]]
        friend std::strong_ordering operator<=>(const CompatIterator& lhs, const CompatIterator& rhs) {
            return std::visit([&]<typename Position>(const Position& position) -> std::strong_ordering {
                if constexpr (std::same_as<Position, object_iterator>)
                    throw ers::make_conversion_error("Can't compare order of JSON object iterators");
                else
                    return position <=> std::get<Position>(rhs._position);
            }, lhs._position);
        }


    private:
        node_pointer _node = nullptr;
        position_type _position {};


        template<class Position>
        CompatIterator(node_pointer node, Position position) :
            _node(node),
            _position(std::move(position)) {
        }
    };
}


// Facade ----------------------------------------------------------------------

namespace utl::impl {
    template<class Node>
    class InjaCompat;


    template<JsonTraits Traits>
    class InjaCompat<TNode<Traits>> {
        using node_type = TNode<Traits>;


    public:
        using string_t = typename Traits::string_type;
        using boolean_t = bool;
        using number_integer_t = typename Traits::integral_type;
        using number_unsigned_t = std::make_unsigned_t<number_integer_t>;
        using number_float_t = typename Traits::floating_type;
        using json_pointer = JsonPointer;

        using iterator = CompatIterator<node_type, false>;
        using const_iterator = CompatIterator<node_type, true>;


        // -- Parsing --
        // -------------

        [[nodiscard]]
        static node_type parse(std::string_view chars) {
            return from_string<node_type>(chars);
        }

        template<std::input_iterator It>
        [[nodiscard]]
        static node_type parse(It first, It last) {
            return parse(std::string(first, last));
        }

        [[nodiscard]]
        std::string dump() const {
            return _self().to_string(EFormat::Minimized);
        }


        // -- Type information --
        // ----------------------

        [[nodiscard]] bool is_boolean() const noexcept { return _self().is_bool(); }
        [[nodiscard]] bool is_number() const noexcept { return _self().is_integral() || _self().is_floating(); }
        [[nodiscard]] bool is_number_integer() const noexcept { return _self().is_integral(); }
        [[nodiscard]] bool is_number_unsigned() const noexcept { return false; }
        [[nodiscard]] bool is_number_float() const noexcept { return _self().is_floating(); }


        // -- Getters --
        // -------------

        template<class T>
        [[nodiscard]] std::remove_cvref_t<T> get() const {
            using value_t = std::remove_cvref_t<T>;
            const auto& node = _self();

            if constexpr (std::same_as<value_t, node_type>) {
                return node;
            } else if constexpr (std::same_as<value_t, bool>) {
                return _checked<bool>();
            } else if constexpr (std::is_arithmetic_v<value_t>) {
                if (const auto* value = node.template get_if<number_integer_t>())
                    return static_cast<value_t>(*value);
                if (const auto* value = node.template get_if<number_float_t>())
                    return static_cast<value_t>(*value);
                throw _type_error("number");
            } else if constexpr (std::same_as<value_t, string_t>) {
                return _checked<string_t>();
            } else if constexpr (std::same_as<value_t, std::string>) {
                return std::string(std::string_view(string_adaptor<string_t>::to_utf8(_checked<string_t>())));
            } else if constexpr (std::same_as<value_t, std::vector<node_type>>) {
                const auto& array = _checked<typename node_type::array_type>();
                return value_t(array.begin(), array.end());
            } else
                static_assert(false, "inja_compat::get doesn't support this type");
        }

        template<class T>
            requires std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>
        [[nodiscard]] T get_ref() const {
            return _checked<std::remove_cvref_t<T>>();
        }


        // -- Container access --
        // ----------------------

        [[nodiscard]] std::size_t size() const noexcept {
            const auto& node = _self();

            switch (node.type()) {
                case ENodeType::None:
                    return 0;

                case ENodeType::Object:
                    return node.as_object().size();

                case ENodeType::Array:
                    return node.as_array().size();

                default:
                    return 1;
            }
        }

        [[nodiscard]] bool empty() const noexcept {
            const auto& node = _self();

            switch (node.type()) {
                case ENodeType::None:
                    return true;

                case ENodeType::Object:
                    return node.as_object().empty();

                case ENodeType::Array:
                    return node.as_array().empty();

                case ENodeType::String:
                    return std::string_view(string_adaptor<string_t>::to_utf8(node.as_string())).empty();

                default:
                    return false;
            }
        }

        void clear() {
            auto& node = _self();

            switch (node.type()) {
                case ENodeType::None:
                    return;

                case ENodeType::Object:
                    node.as_object().clear();
                    return;

                case ENodeType::Array:
                    node.as_array().clear();
                    return;

                case ENodeType::String:
                    node.as_string() = string_t {};
                    return;

                case ENodeType::Integral:
                    node.as_integral() = 0;
                    return;

                case ENodeType::Floating:
                    node.as_floating() = 0;
                    return;

                case ENodeType::Bool:
                    node.as_bool() = false;
                    return;
            }

            std::unreachable();
        }


        [[nodiscard]] node_type& front() {
            return const_cast<node_type&>(std::as_const(*this).front());
        }
        [[nodiscard]] const node_type& front() const {
            if (size() == 0)
                throw ers::make_out_of_range_error("Can't take front() of an empty JSON {}", type_label(_self().type()));
            return *begin();
        }

        [[nodiscard]] const node_type& back() const {
            if (size() == 0)
                throw ers::make_out_of_range_error("Can't take back() of an empty JSON {}", type_label(_self().type()));
            return *std::prev(end());
        }
        [[nodiscard]] node_type& back() {
            return const_cast<node_type&>(std::as_const(*this).back());
        }


        [[nodiscard]] iterator find(std::string_view key) {
            auto& node = _self();
            auto* object = node.template get_if<typename node_type::object_type>();
            return object ? iterator::from(node, json_detail::find_key(*object, key)) : end();
        }
        [[nodiscard]] const_iterator find(std::string_view key) const {
            const auto& node = _self();
            const auto* object = node.template get_if<typename node_type::object_type>();
            return object ? const_iterator::from(node, json_detail::find_key(*object, key)) : end();
        }


        [[nodiscard]] iterator begin() { return iterator::begin_of(_self()); }
        [[nodiscard]] iterator end() { return iterator::end_of(_self()); }

        [[nodiscard]] const_iterator begin() const { return const_iterator::begin_of(_self()); }
        [[nodiscard]] const_iterator end() const { return const_iterator::end_of(_self()); }

        [[nodiscard]] const_iterator cbegin() const { return begin(); }
        [[nodiscard]] const_iterator cend() const { return end(); }


        // -- Comparison --
        // ----------------

        [[nodiscard]]
        friend bool operator==(const node_type& lhs, const node_type& rhs) {
            return _equal(lhs, rhs);
        }

        [[nodiscard]]
        friend std::partial_ordering operator<=>(const node_type& lhs, const node_type& rhs) {
            return _compare(lhs, rhs);
        }


    private:
        [[nodiscard]] node_type& _self() noexcept { return static_cast<node_type&>(*this); }
        [[nodiscard]] const node_type& _self() const noexcept { return static_cast<const node_type&>(*this); }


        template<class T>
        [[nodiscard]] const T& _checked() const {
            if (const auto* value = _self().template get_if<T>())
                return *value;

            if constexpr (std::same_as<T, bool>)
                throw _type_error("bool");
            else if constexpr (std::same_as<T, string_t>)
                throw _type_error("string");
            else
                throw _type_error("array");
        }

        [[nodiscard]]
        auto _type_error(std::string_view expected) const {
            return ers::make_conversion_error("JSON {} isn't a {}",
                type_label(_self().type()), expected);
        }


        [[nodiscard]]
        static int _type_rank(ENodeType type) noexcept {
            switch (type) {
                case ENodeType::None:
                    return 0;

                case ENodeType::Bool:
                    return 1;

                case ENodeType::Integral:
                case ENodeType::Floating:
                    return 2;

                case ENodeType::Object:
                    return 3;

                case ENodeType::Array:
                    return 4;

                case ENodeType::String:
                    return 5;
            }

            return 0;
        }

        [[nodiscard]]
        static std::string_view _utf8(const string_t& value) {
            return std::string_view(string_adaptor<string_t>::to_utf8(value));
        }

        template<class Key>
        [[nodiscard]]
        static std::string_view _key_utf8(const Key& key) {
            return std::string_view(string_adaptor<Key>::to_utf8(key));
        }


        [[nodiscard]]
        static bool _equal(const node_type& lhs, const node_type& rhs) {
            if (lhs.is_array() && rhs.is_array()) {
                const auto& left = lhs.as_array();
                const auto& right = rhs.as_array();
                
                return std::ranges::equal(left, right, _equal);
            }

            if (lhs.is_object() && rhs.is_object()) {
                const auto& left = lhs.as_object();
                const auto& right = rhs.as_object();

                if (left.size() != right.size())
                    return false;

                for (const auto& [key, value] : left) {
                    const auto it = json_detail::find_key(right, _key_utf8(key));
                    if (it == right.end() || !_equal(value, it->second))
                        return false;
                }

                return true;
            }

            return _compare(lhs, rhs) == std::partial_ordering::equivalent;
        }

        [[nodiscard]]
        static std::partial_ordering _compare(const node_type& lhs, const node_type& rhs) {
            if (lhs.is_number() && rhs.is_number()) {
                if (lhs.is_integral() && rhs.is_integral())
                    return lhs.as_integral() <=> rhs.as_integral();
                return lhs.template get<number_float_t>() <=> rhs.template get<number_float_t>();
            }

            if (const auto order = _type_rank(lhs.type()) <=> _type_rank(rhs.type()); order != 0)
                return order;

            switch (lhs.type()) {
                case ENodeType::Bool:
                    return lhs.as_bool() <=> rhs.as_bool();

                case ENodeType::String:
                    return _utf8(lhs.as_string()) <=> _utf8(rhs.as_string());

                case ENodeType::Array: {
                    const auto& left = lhs.as_array();
                    const auto& right = rhs.as_array();

                    return std::lexicographical_compare_three_way(
                        left.begin(), left.end(),
                        right.begin(), right.end(),
                        _compare
                    );
                }

                case ENodeType::Object: {
                    const auto& left = lhs.as_object();
                    const auto& right = rhs.as_object();

                    return std::lexicographical_compare_three_way(
                        left.begin(), left.end(),
                        right.begin(), right.end(),
                        [](const auto& a, const auto& b) -> std::partial_ordering {
                            if (const auto order = _key_utf8(a.first) <=> _key_utf8(b.first); order != 0)
                                return order;
                            return _compare(a.second, b.second);
                        }
                    );
                }

                default:
                    return std::partial_ordering::equivalent;
            }
        }
    };
}


// Traits ----------------------------------------------------------------------

namespace utl::impl {
    struct inja_traits : ordered_traits {
        static constexpr bool assign_by_value = true;


        template<typename Node>
        using extension_type = InjaCompat<Node>;
    };

    using InjaJson = TNode<inja_traits>;
}

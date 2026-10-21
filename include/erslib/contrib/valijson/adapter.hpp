#pragma once

// std
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// valijson
#include <valijson/exceptions.hpp>
#include <valijson/internal/adapter.hpp>
#include <valijson/internal/basic_adapter.hpp>
#include <valijson/internal/frozen_value.hpp>

// ers
#include <erslib/contrib/json/impl.hpp>


namespace utl::impl::valijson_adapter {
    template<typename Node>
    class Adapter;

    template<typename Node>
    using Member = std::pair<std::string, Adapter<Node>>;


    template<typename It>
    using category_of = std::conditional_t<std::bidirectional_iterator<It>,
        std::bidirectional_iterator_tag, std::forward_iterator_tag>;
}


namespace utl::impl::valijson_adapter {
    template<typename Node>
    class ArrayIterator {
        using base_iterator = typename Node::array_type::const_iterator;


    public:
        using iterator_category = category_of<base_iterator>;
        using value_type = Adapter<Node>;
        using difference_type = Adapter<Node>;
        using pointer = Adapter<Node>*;
        using reference = Adapter<Node>&;


        ArrayIterator(base_iterator it) :
            _it(it) {
        }


        Adapter<Node> operator*() const { return Adapter<Node>(*_it); }
        valijson::adapters::DerefProxy<Adapter<Node>> operator->() const { return valijson::adapters::DerefProxy(**this); }


        bool operator==(const ArrayIterator& other) const { return _it == other._it; }
        

        const ArrayIterator& operator++() {
            ++_it;
            return *this;
        }
        ArrayIterator operator++(int) {
            auto out = *this;
            ++_it;
            return out;
        }

        const ArrayIterator& operator--() requires std::bidirectional_iterator<base_iterator> {
            --_it;
            return *this;
        }


        void advance(std::ptrdiff_t n) { std::ranges::advance(_it, n); }


    private:
        base_iterator _it;
    };


    template<typename Node>
    class Array {
    public:
        using const_iterator = ArrayIterator<Node>;
        using iterator = const_iterator;


        Array() : _array(empty()) {}
        explicit Array(const Node& value) : _array(value.as_array()) {}


        const_iterator begin() const { return _array.begin(); }
        const_iterator end() const { return _array.end(); }
        size_t size() const { return std::ranges::size(_array); }


    private:
        const typename Node::array_type& _array;


        static const typename Node::array_type& empty() {
            static const typename Node::array_type out;
            return out;
        }
    };
}


namespace utl::impl::valijson_adapter {
    template<typename Node>
    class MemberIterator {
        using base_iterator = typename Node::object_type::const_iterator;


    public:
        using iterator_category = category_of<base_iterator>;
        using value_type = Member<Node>;
        using difference_type = Member<Node>;
        using pointer = Member<Node>*;
        using reference = Member<Node>&;


        MemberIterator(base_iterator it) :
            _it(it) {
        }


        Member<Node> operator*() const {
            return Member<Node>(std::string(_it->first), Adapter<Node>(_it->second));
        }

        valijson::adapters::DerefProxy<Member<Node>> operator->() const { return valijson::adapters::DerefProxy(**this); }


        bool operator==(const MemberIterator& other) const { return _it == other._it; }
        

        const MemberIterator& operator++() {
            ++_it;
            return *this;
        }
        MemberIterator operator++(int) {
            auto out = *this;
            ++_it;
            return out;
        }


        const MemberIterator& operator--() requires std::bidirectional_iterator<base_iterator> {
            --_it;
            return *this;
        }


    private:
        base_iterator _it;
    };


    template<typename Node>
    class Object {
        using object_type = typename Node::object_type;


    public:
        using const_iterator = MemberIterator<Node>;
        using iterator = const_iterator;


        Object() : _object(empty()) {}
        explicit Object(const Node& value) : _object(value.as_object()) {}


        const_iterator begin() const { return _object.begin(); }
        const_iterator end() const { return _object.end(); }
        size_t size() const { return _object.size(); }

        const_iterator find(const std::string& name) const {
            if constexpr (requires(const object_type& o, std::string_view k) { o.find(k); })
                return _object.find(name);
            else
                return _object.find(typename object_type::key_type(name));
        }


    private:
        const object_type& _object;


        static const object_type& empty() {
            static const object_type out;
            return out;
        }
    };
}


namespace utl::impl::valijson_adapter {
    template<typename Node>
    class FrozenValue final : public valijson::adapters::FrozenValue {
    public:
        explicit FrozenValue(Node value) : _value(std::move(value)) {}

        valijson::adapters::FrozenValue* clone() const override { return new FrozenValue(_value); }

        bool equalTo(const valijson::adapters::Adapter& other, bool strict) const override {
            return Adapter<Node>(_value).equalTo(other, strict);
        }


    private:
        Node _value;
    };


    template<typename Node>
    class Value {
    public:
        Value() : _value(empty()) {}
        Value(const Node& value) : _value(value) {}


        valijson::adapters::FrozenValue* freeze() const { return new FrozenValue<Node>(_value); }

        std::optional<Array<Node>> getArrayOptional() const {
            return _value.is_array() ? std::optional(Array<Node>(_value)) : std::nullopt;
        }

        std::optional<Object<Node>> getObjectOptional() const {
            return _value.is_object() ? std::optional(Object<Node>(_value)) : std::nullopt;
        }

        bool getArraySize(size_t& out) const {
            return _value.is_array() && (out = std::ranges::size(_value.as_array()), true);
        }

        bool getObjectSize(size_t& out) const {
            return _value.is_object() && (out = _value.as_object().size(), true);
        }

        bool getBool(bool& out) const {
            return _value.is_bool() && (out = _value.as_bool(), true);
        }

        bool getInteger(int64_t& out) const {
            if (!_value.is_integral() || !std::in_range<int64_t>(_value.as_integral()))
                return false;

            out = static_cast<int64_t>(_value.as_integral());
            return true;
        }

        bool getDouble(double& out) const {
            return _value.is_floating() && (out = static_cast<double>(_value.as_floating()), std::isfinite(out));
        }

        bool getString(std::string& out) const {
            return _value.is_string() && (out.assign(_value.as_string()), true);
        }


        static bool hasStrictTypes() { return true; }

        bool isArray() const { return _value.is_array(); }
        bool isObject() const { return _value.is_object(); }
        bool isString() const { return _value.is_string(); }
        bool isBool() const { return _value.is_bool(); }
        bool isNull() const { return _value.is_null(); }
        bool isInteger() const { return _value.is_integral() && std::in_range<int64_t>(_value.as_integral()); }
        bool isDouble() const { return _value.is_floating() && std::isfinite(static_cast<double>(_value.as_floating())); }
        bool isNumber() const { return isInteger() || isDouble(); }


    private:
        static const Node& empty() {
            static const Node out = Node(typename Node::object_type {});
            return out;
        }

        const Node& _value;
    };


    template<typename Node>
    class Adapter : public valijson::adapters::BasicAdapter<Adapter<Node>, Array<Node>, Member<Node>, Object<Node>, Value<Node>> {
        using base = valijson::adapters::BasicAdapter<Adapter<Node>, Array<Node>, Member<Node>, Object<Node>, Value<Node>>;


    public:
        Adapter() = default;
        Adapter(const Node& value) : base(Value<Node>(value)) {}
    };
}


template<typename Traits>
struct valijson::adapters::AdapterTraits<utl::impl::valijson_adapter::Adapter<utl::impl::TNode<Traits>>> {
    using DocumentType = utl::impl::TNode<Traits>;

    static std::string adapterName() { return "UtlJsonAdapter"; }
};

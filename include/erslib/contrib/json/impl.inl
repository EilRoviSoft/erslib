#pragma once


// Helpers ---------------------------------------------------------------------

namespace utl::impl::json_detail {
    template<class Key>
    [[nodiscard]]
    Key make_key(std::string_view key) {
        return string_adaptor<Key>::from_utf8(std::string(key));
    }

    template<class Object>
    [[nodiscard]]
    auto find_key(Object& object, std::string_view key) {
        if constexpr (requires { object.find(key); })
            return object.find(key);
        else
            return object.find(make_key<typename std::remove_const_t<Object>::key_type>(key));
    }
}


// Conversion ------------------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    template<class T>
        requires json_detail::ConvertibleSource<T, TNode<Traits>>
    TNode<Traits>::TNode(T&& value) {
        using source_t = std::remove_cvref_t<T>;
        using json_detail::ESourceKind;

        constexpr auto kind = json_detail::classify<T, TNode>();

        if constexpr (kind == ESourceKind::OtherNode) {
            std::visit([this]<typename Alternative>(Alternative&& alternative) {
                *this = TNode(std::forward<Alternative>(alternative));
            }, std::forward_like<T>(value._data));
        } else if constexpr (kind == ESourceKind::Null) {
        } else if constexpr (kind == ESourceKind::Boolean) {
            _data.template emplace<bool_type>(value);
        } else if constexpr (kind == ESourceKind::String) {
            _data.template emplace<string_type>(json_detail::to_string_type<string_type>(std::forward<T>(value)));
        } else if constexpr (kind == ESourceKind::Integral) {
            _data.template emplace<integral_type>(checked_integral(value));
        } else if constexpr (kind == ESourceKind::Floating) {
            _data.template emplace<floating_type>(static_cast<floating_type>(value));
        } else if constexpr (kind == ESourceKind::Optional) {
            if (value.has_value())
                *this = TNode(std::forward_like<T>(*value));
        } else if constexpr (kind == ESourceKind::Object) {
            if constexpr (std::same_as<source_t, object_type>) {
                _data.template emplace<object_type>(std::forward<T>(value));
            } else {
                using key_type = typename object_type::key_type;

                auto& object = _data.template emplace<object_type>();
                if constexpr (std::ranges::sized_range<source_t> && requires { object.reserve(std::size_t {}); })
                    object.reserve(std::ranges::size(value));

                for (auto&& [key, item] : value)
                    object.try_emplace(json_detail::to_string_type<key_type>(key), std::forward_like<T>(item));
            }
        } else if constexpr (kind == ESourceKind::Array) {
            if constexpr (std::same_as<source_t, array_type>) {
                _data.template emplace<array_type>(std::forward<T>(value));
            } else {
                auto& array = _data.template emplace<array_type>();
                if constexpr (std::ranges::sized_range<source_t> && requires { array.reserve(std::size_t {}); })
                    array.reserve(std::ranges::size(value));

                for (auto&& item : value)
                    array.emplace_back(std::forward_like<T>(item));
            }
        }
    }
}


// Object access ---------------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](std::string_view key) -> TNode& {
        if (is_null())
            _data.template emplace<object_type>();

        auto* object = get_if<object_type>();
        if (!object) {
            throw ers::make_conversion_error("Can't access key '{}' of a JSON {}",
                key, ers::enum_utils::to_string(type()));
        }

        if (auto it = json_detail::find_key(*object, key); it != object->end())
            return it->second;

        return object->try_emplace(json_detail::make_key<typename object_type::key_type>(key)).first->second;
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](std::string_view key) const -> const TNode& {
        return at(key);
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::at(std::string_view key) -> TNode& {
        return const_cast<TNode&>(std::as_const(*this).at(key));
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::at(std::string_view key) const -> const TNode& {
        if (!is_object()) {
            throw ers::make_conversion_error("Can't access key '{}' of a JSON {}",
                key, ers::enum_utils::to_string(type()));
        }

        if (const auto* child = find_child(key))
            return *child;

        throw ers::make_out_of_range_error("JSON object has no key '{}'", key);
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(std::string_view key) -> ers::maybe<TNode&> {
        if (auto* child = find_child(key))
            return *child;
        return ers::none;
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(std::string_view key) const -> ers::maybe<const TNode&> {
        if (const auto* child = find_child(key))
            return *child;
        return ers::none;
    }


    template<JsonTraits Traits>
    bool TNode<Traits>::contains(std::string_view key) const {
        return find_child(key) != nullptr;
    }


    template<JsonTraits Traits>
    template<typename T, typename U>
        requires json_detail::FallbackFor<T, U, TNode<Traits>>
    auto TNode<Traits>::value_or(std::string_view key, U&& fallback) const -> json_detail::value_or_t<T, U, TNode> {
        using value_type = json_detail::value_or_t<T, U, TNode>;

        if (!is_object()) {
            throw ers::make_conversion_error("Can't access key '{}' of a JSON {}",
                key, ers::enum_utils::to_string(type()));
        }

        if (const auto* child = find_child(key); child && !child->is_null())
            return read_value<value_type>(*child, key);

        return read_value<value_type>(TNode(std::forward<U>(fallback)), key);
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::find_child(std::string_view key) -> TNode* {
        return const_cast<TNode*>(std::as_const(*this).find_child(key));
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::find_child(std::string_view key) const -> const TNode* {
        const auto* object = get_if<object_type>();
        if (!object)
            return nullptr;

        const auto it = json_detail::find_key(*object, key);
        return it != object->end() ? &it->second : nullptr;
    }
}


// Array access ----------------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](std::size_t index) -> TNode& {
        return array_or_throw(index)[index];
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](std::size_t index) const -> const TNode& {
        return array_or_throw(index)[index];
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::at(std::size_t index) -> TNode& {
        return const_cast<TNode&>(std::as_const(*this).at(index));
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::at(std::size_t index) const -> const TNode& {
        const auto& array = array_or_throw(index);

        if (index >= array.size()) {
            throw ers::make_out_of_range_error("Index {} is out of range for a JSON array of size {}",
                index, array.size());
        }

        return array[index];
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(std::size_t index) -> ers::maybe<TNode&> {
        auto* array = get_if<array_type>();
        if (!array || index >= array->size())
            return ers::none;
        return (*array)[index];
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(std::size_t index) const -> ers::maybe<const TNode&> {
        const auto* array = get_if<array_type>();
        if (!array || index >= array->size())
            return ers::none;
        return (*array)[index];
    }


    template<JsonTraits Traits>
    void TNode<Traits>::push_back(const TNode& value) {
        emplace_back(value);
    }

    template<JsonTraits Traits>
    void TNode<Traits>::push_back(TNode&& value) {
        emplace_back(std::move(value));
    }

    template<JsonTraits Traits>
    template<class... Args>
        requires std::constructible_from<TNode<Traits>, Args...>
    auto TNode<Traits>::emplace_back(Args&&... args) -> TNode& {
        if (is_null())
            _data.template emplace<array_type>();

        auto* array = get_if<array_type>();
        if (!array) {
            throw ers::make_conversion_error("Can't append to a JSON {}",
                ers::enum_utils::to_string(type()));
        }

        array->emplace_back(std::forward<Args>(args)...);
        return array->back();
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::array_or_throw(std::size_t index) -> array_type& {
        return const_cast<array_type&>(std::as_const(*this).array_or_throw(index));
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::array_or_throw(std::size_t index) const -> const array_type& {
        const auto* array = get_if<array_type>();
        if (!array) {
            throw ers::make_conversion_error("Can't access index {} of a JSON {}",
                index, ers::enum_utils::to_string(type()));
        }

        return *array;
    }
}


// Pointer access --------------------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](const JsonPointer& pointer) -> TNode& {
        auto* node = this;

        for (const auto& token : pointer.tokens()) {
            if (!node->is_array()) {
                node = &(*node)[token];
                continue;
            }

            if (token == "-") {
                node = &node->emplace_back();
                continue;
            }

            const auto index = JsonPointer::array_index(token);
            if (!index) {
                throw ers::make_conversion_error("'{}' isn't an index of a JSON array",
                    token);
            }

            node = *index == node->as_array().size()
                ? &node->emplace_back()
                : &node->at(*index);
        }

        return *node;
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::operator[](const JsonPointer& pointer) const -> const TNode& {
        return at(pointer);
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::at(const JsonPointer& pointer) -> TNode& {
        return const_cast<TNode&>(std::as_const(*this).at(pointer));
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::at(const JsonPointer& pointer) const -> const TNode& {
        const auto* node = this;

        for (const auto& token : pointer.tokens()) {
            if (!node->is_array()) {
                node = &node->at(token);
                continue;
            }

            const auto index = JsonPointer::array_index(token);
            if (!index)
                throw ers::make_conversion_error("'{}' isn't an index of a JSON array", token);

            node = &node->at(*index);
        }

        return *node;
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(const JsonPointer& pointer) -> ers::maybe<TNode&> {
        if (const auto* node = find_pointer(pointer))
            return const_cast<TNode&>(*node);
        return ers::none;
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(const JsonPointer& pointer) const -> ers::maybe<const TNode&> {
        if (const auto* node = find_pointer(pointer))
            return *node;
        return ers::none;
    }


    template<JsonTraits Traits>
    bool TNode<Traits>::contains(const JsonPointer& pointer) const {
        return find_pointer(pointer) != nullptr;
    }


    template<JsonTraits Traits>
    auto TNode<Traits>::find_pointer(const JsonPointer& pointer) const -> const TNode* {
        const auto* node = this;

        for (const auto& token : pointer.tokens()) {
            if (const auto* array = node->template get_if<array_type>()) {
                const auto index = JsonPointer::array_index(token);
                if (!index || *index >= array->size())
                    return nullptr;

                node = &(*array)[*index];
            } else {
                node = node->find_child(token);
                if (!node)
                    return nullptr;
            }
        }

        return node;
    }
}


// Internal implementation -----------------------------------------------------

namespace utl::impl {
    template<JsonTraits Traits>
    template<std::integral T>
    auto TNode<Traits>::checked_integral(T value) -> integral_type {
        using integer_t = std::conditional_t<std::is_signed_v<T>, std::make_signed_t<T>, std::make_unsigned_t<T>>;

        if (!std::in_range<integral_type>(static_cast<integer_t>(value))) {
            throw ers::make_out_of_range_error("Integer {} doesn't fit into the JSON integral type",
                value);
        }

        return static_cast<integral_type>(value);
    }

    template<JsonTraits Traits>
    template<typename V>
    V TNode<Traits>::read_value(const TNode& node, std::string_view key) {
        if constexpr (std::same_as<V, TNode>) {
            return node;
        } else if constexpr (std::same_as<V, bool_type>) {
            if (const auto* value = node.get_if<bool_type>())
                return *value;
        } else if constexpr (std::integral<V>) {
            if (const auto* value = node.get_if<integral_type>()) {
                if (!std::in_range<V>(*value)) {
                    throw ers::make_out_of_range_error("JSON key '{}' holds {}, which doesn't fit into the requested type",
                        key, *value);
                }

                return static_cast<V>(*value);
            }
        } else if constexpr (std::floating_point<V>) {
            if (const auto* value = node.get_if<floating_type>())
                return static_cast<V>(*value);

            if (const auto* value = node.get_if<integral_type>())
                return static_cast<V>(*value);
        } else {
            if (const auto* value = node.get_if<V>())
                return *value;
        }

        throw ers::make_conversion_error("JSON key '{}' is {}, which can't be read as the requested type",
            key, ers::enum_utils::to_string(node.type()));
    }
}

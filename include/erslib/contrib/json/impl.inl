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
    auto TNode<Traits>::try_at(std::string_view key) -> ers::optional<TNode&> {
        if (auto* child = find_child(key))
            return *child;
        return ers::nullopt;
    }

    template<JsonTraits Traits>
    auto TNode<Traits>::try_at(std::string_view key) const -> ers::optional<const TNode&> {
        if (const auto* child = find_child(key))
            return *child;
        return ers::nullopt;
    }


    template<JsonTraits Traits>
    bool TNode<Traits>::contains(std::string_view key) const {
        return find_child(key) != nullptr;
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

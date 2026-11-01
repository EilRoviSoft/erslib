#pragma once

// std
#include <vector>

// ers
#include <erslib/contrib/json/value.hpp>


template<typename T>
struct utl::impl::json_value<std::vector<T>> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, std::vector<T>> read(const Node& json) {
        if (!json.is_array())
            return wrong_type<std::vector<T>, Policy>("array", json);

        const auto& array = json.as_array();

        std::vector<T> out;
        out.reserve(array.size());

        for (size_t i = 0; i < array.size(); ++i) {
            auto item = json_value<T>::template read<Policy>(array[i]);

            if (!item)
                return Policy::template propagate<std::vector<T>>(std::move(item), "[{}]", i);

            out.push_back(std::move(*item));
        }

        return out;
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const std::vector<T>& what) {
        out = typename Node::array_type {};

        size_t index = 0;

        for (const auto& value : what) {
            Node item;

            auto status = json_value<T>::template write<Policy>(item, value);

            if (!status)
                return Policy::template propagate<void>(std::move(status), "[{}]", index);

            out.push_back(std::move(item));
            ++index;
        }

        return Policy::done();
    }
};

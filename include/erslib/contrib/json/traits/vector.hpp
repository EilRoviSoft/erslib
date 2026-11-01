#pragma once

// std
#include <vector>

// ers
#include <erslib/contrib/json/value.hpp>


template<typename T>
struct utl::impl::json_value<std::vector<T>> {
    template<JsonNode Node>
    static ers::Result<std::vector<T>> read(const Node& json) {
        if (!json.is_array())
            return expected_error("array", json);

        const auto& array = json.as_array();

        std::vector<T> out;
        out.reserve(array.size());

        for (size_t i = 0; i < array.size(); ++i) {
            auto item = json_value<T>::read(array[i])
                .context("[{}]", i);

            if (!item)
                return item.error();

            out.push_back(std::move(*item));
        }

        return out;
    }

    template<JsonNode Node>
    static void write(Node& out, const std::vector<T>& what) {
        out = typename Node::array_type {};

        for (const auto& value : what) {
            Node item;
            json_value<T>::write(item, value);
            out.push_back(std::move(item));
        }
    }
};

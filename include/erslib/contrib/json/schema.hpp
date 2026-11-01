#pragma once

// std
#include <format>
#include <functional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/contrib/json/traits.hpp>
#include <erslib/core/concept/json.hpp>
#include <erslib/core/formatter/range.hpp>
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    template<typename Arg, typename Node>
    using schema_input_t = std::conditional_t<
        StringLike<std::remove_cvref_t<Arg>>,
        std::string,
        json_storage_t<Arg, Node>
    >;


    template<JsonNode Node>
    class TSchema {
        template<typename F>
        using extract_return_type = ers::result_traits<typename ers::fn_traits<F>::return_type>::value_type;


    public:
        explicit TSchema(const Node& json) :
            m_json(json) {
        }


        template<JsonConvertible T>
        void require(std::string_view name) {
            _read<T>(name, true);
        }


        template<JsonConvertible T>
        void write_if_exist(std::string_view name, T& out) {
            _write(name, false, out);
        }

        template<JsonConvertible T>
        void require_and_write(std::string_view name, T& out) {
            _write(name, true, out);
        }

        template<typename F>
        void require_and_convert(std::string_view name, extract_return_type<F>& out, F&& writer) {
            using input_t = schema_input_t<typename ers::fn_traits<F>::template arg_type<0>, Node>;

            auto input = _read<input_t>(name, true);

            if (!input)
                return;

            if (auto r = writer(*input); r) {
                m_moves.push_back([&out, value = std::move(*r)] mutable {
                    out = std::move(value);
                    });
            } else
                m_errors.push_back(std::format("field '{}': {}", name, r.error().to_string(true)));
        }


        [[nodiscard]]
        ers::Status finalize() {
            if (!m_errors.empty()) {
                return ers::make_error("{}",
                    std::views::join_with(m_errors, "; "));
            }

            for (auto& move : m_moves)
                move();

            return ers::ok;
        }


    protected:
        const Node& m_json;
        std::vector<std::string> m_errors;
        std::vector<std::move_only_function<void()>> m_moves;


    private:
        const Node* _field(std::string_view name, bool required) {
            if (m_json.is_object()) {
                if (auto node = m_json.try_at(name))
                    return &*node;
            }

            if (required)
                m_errors.push_back(std::format("no field '{}'", name));

            return nullptr;
        }

        template<typename T>
        ers::maybe<T> _read(std::string_view name, bool required) {
            const auto* node = _field(name, required);

            if (!node)
                return ers::none;

            auto value = json_value<T>::read(*node);

            if (!value) {
                m_errors.push_back(std::format("field '{}': {}", name, value.error().to_string(true)));
                return ers::none;
            }

            return ers::maybe<T> { std::move(*value) };
        }

        template<typename T>
        void _write(std::string_view name, bool required, T& out) {
            if (auto value = _read<T>(name, required)) {
                m_moves.push_back([&out, value = std::move(*value)] mutable {
                    out = std::move(value);
                });
            }
        }
    };


    using JsonSchema = TSchema<Json>;
}

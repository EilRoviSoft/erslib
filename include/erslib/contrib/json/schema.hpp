#pragma once

// std
#include <concepts>
#include <functional>
#include <ranges>
#include <vector>

// ers
#include <erslib/contrib/json/concept.hpp>
#include <erslib/contrib/json/convert.hpp>
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/concept/json.hpp>
#include <erslib/core/formatter/range.hpp>
#include <erslib/core/meta/type_name.hpp>
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    class JsonSchema {
        using json_iterator = Json::object_type::const_iterator;


        template<typename F>
        using extract_return_type = ers::result_traits<typename ers::fn_traits<F>::return_type>::value_type;


    public:
        explicit JsonSchema(const Json& json) :
            m_json(json) {
        }


        template<JsonCompatible T>
        void require(std::string_view name) {
            _typed<T>(name, true);
        }


        template<JsonCompatible T>
        void write_if_exist(std::string_view name, T& out) {
            if (const auto* node = _typed<T>(name, false))
                _defer(out, *node);
        }


        template<JsonCompatible T>
        void require_and_write(std::string_view name, T& out) {
            if (const auto* node = _typed<T>(name, true))
                _defer(out, *node);
        }

        template<typename F>
        void require_and_convert(std::string_view name, extract_return_type<F>& out, F&& writer) {
            using json_type = json_conversion<typename ers::fn_traits<F>::template arg_type<0>>::original_type;

            if (const auto* node = _typed<json_type>(name, true)) {
                if (auto r = writer(node->template as<json_type>()); r) {
                    m_moves.emplace_back([&out, value = std::move(*r)] mutable {
                        out = std::move(value);
                    });
                } else
                    m_errors.push_back(std::format("field '{}': {}", name, r.error().message()));
            }
        }

        template<std::derived_from<IJsonConvertible> T>
        void require_and_load(std::string_view name, T& out) {
            if (const auto* node = _field(name, true))
                _load(name, *node, out);
        }

        template<std::derived_from<IJsonConvertible> T>
        void load_if_exist(std::string_view name, T& out) {
            if (const auto* node = _field(name, false))
                _load(name, *node, out);
        }



        [[nodiscard]]
        ers::Status finalize() {
            if (!m_errors.empty()) {
                return ers::make_error("{}",
                    std::views::join_with(m_errors, "; "));
            }

            for (const auto& it : m_writes)
                it.apply(it.out, *it.value);

            for (auto& move : m_moves)
                move();

            return ers::ok;
        }


    protected:
        struct write_t {
            void* out;
            const Json* value;
            void (*apply)(void*, const Json&);
        };


        const Json& m_json;
        std::vector<std::string> m_errors;
        std::vector<write_t> m_writes;
        std::vector<std::move_only_function<void()>> m_moves;


    private:
        template<typename T>
        void _defer(T& out, const Json& value) {
            m_writes.push_back({
                &out, &value, [](void* out, const Json& value) {
                    *static_cast<T*>(out) = value.template as<T>();
                }
            });
        }


        template<JsonCompatible T>
        const Json* _typed(std::string_view name, bool required) {
            const auto& object = m_json.as_object();
            auto it = object.find(name);

            if (it == object.end()) {
                if (required)
                    m_errors.push_back(std::format("no field '{}'", name));
                return nullptr;
            }

            if (!it->second.template is<T>()) {
                m_errors.push_back(std::format("field '{}' isn't '{}'", name, ers::meta::type_name_v<T>));
                return nullptr;
            }

            return &it->second;
        }


        const Json* _field(std::string_view name, bool required) {
            const auto& object = m_json.as_object();
            auto it = object.find(name);

            if (it == object.end()) {
                if (required)
                    m_errors.push_back(std::format("no field '{}'", name));
                return nullptr;
            }

            return &it->second;
        }


        template<std::derived_from<IJsonConvertible> T>
        void _load(std::string_view name, const Json& node, T& out) {
            T value;

            if (auto s = value.load_from(node); !s) {
                m_errors.push_back(std::format("field '{}': {}", name, s.error().message()));
                return;
            }

            m_moves.emplace_back([&out, value = std::move(value)] mutable {
                out = std::move(value);
            });
        }
    };
}

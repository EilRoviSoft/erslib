#pragma once

// std
#include <functional>
#include <list>

// ers
#include <erslib/contrib/json/concept.hpp>
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/concept/json.hpp>
#include <erslib/core/meta/type_name.hpp>
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    class JsonSchema {
        using json_iterator = Node::object_type::const_iterator;


        template<typename F>
        using extract_return_type = ers::result_traits<typename ers::fn_traits<F>::return_type>::value_type;


    public:
        explicit JsonSchema(const Node& json) :
            m_json(json) {
        }


        template<JsonCompatible T>
        void require(std::string_view name) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); !r)
                m_error = std::move(r.error());
        }


        template<JsonCompatible T>
        void write_if_exist(std::string_view name, T& out) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); r) {
                m_assignments.emplace_back([&out, value = (*r)->second] {
                    out = value.template as<T>();
                });
            }
        }


        template<JsonCompatible T>
        void require_and_write(std::string_view name, T& out) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); r) {
                m_assignments.emplace_back([&out, value = (*r)->second] {
                    out = value.template as<T>();
                });
            } else
                m_error = std::move(r.error());
        }

        template<typename F>
        void require_and_convert(std::string_view name, extract_return_type<F>& out, F&& writer) {
            using arg_type = ers::fn_traits<F>::template arg_type<0>;
            using json_type = json_conversion<arg_type>::original_type;


            static_assert(ers::fn_traits<F>::arity == 1, "'F' should have only 1 argument");
            static_assert(ers::is_result_v<typename ers::fn_traits<F>::return_type>, "Return type of 'F' should be 'ers::Result'");


            const auto& object = m_json.as_object();

            if (auto r = _check<json_type>(object, name); r) {
                m_assignments.emplace_back([&out, writer = std::move(writer), value = (*r)->second] {
                    if (auto r = writer(value.template as<json_type>()); r)
                        out = *r;
                    else
                        throw conversion_error(r.error().to_string(true));
                });
            } else
                m_error = r.error();
        }


        [[nodiscard]]
        ers::Status finalize() const {
            if (m_error)
                return *m_error;

            for (const auto& assign : m_assignments)
                assign();

            return ers::ok;
        }


    protected:
        const Node& m_json;
        std::optional<ers::Diagnostic> m_error;
        std::list<std::function<void()>> m_assignments;


    private:
        template<JsonCompatible T>
        static ers::Result<json_iterator> _check(const Node::object_type& object, std::string_view name) {
            auto it = object.find(name);

            if (it == object.end()) {
                return ers::make_error("Json doesn't have field with name \"{}\"",
                    name);
            }

            if (!it->second.is<T>()) {
                return make_error("Field with name \"{}\" has mismatched type \"{}\"",
                    name, ers::meta::type_name_v<T>);
            }

            return it;
        }
    };
}

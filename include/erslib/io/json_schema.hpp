#pragma once

// std
#include <functional>
#include <list>

// ers
#include <contrib/json.hpp>
#include <erslib/concept/json.hpp>
#include <erslib/trait/fn.hpp>
#include <erslib/trait/result.hpp>
#include <erslib/type/result.hpp>


namespace ers {
    class JsonSchema {
        using json_iterator = utl::Json::object_type::const_iterator;


        template<typename F>
        using extract_return_type = result_traits<typename fn_traits<F>::return_type>::value_type;


    public:
        explicit JsonSchema(const utl::Json& json) :
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
            using arg_type = fn_traits<F>::template arg_type<0>;
            using json_type = utl::internal::json_conversion<arg_type>::original_type;


            static_assert(fn_traits<F>::arity == 1, "'F' should have only 1 argument");
            static_assert(ers::is_result_v<typename fn_traits<F>::return_type>, "Return type of 'F' should be 'ers::Result'");


            const auto& object = m_json.as_object();

            if (auto r = _check<json_type>(object, name); r) {
                m_assignments.emplace_back([&out, writer = std::move(writer), value = (*r)->second] {
                    if (auto r = writer(value.template as<json_type>()); r)
                        out = *r;
                    else
                        throw Exception(r.error());
                });
            } else
                m_error = r.error();
        }


        [[nodiscard]]
        Status finalize() const {
            if (m_error)
                return *m_error;

            for (const auto& assign : m_assignments)
                assign();

            return ok;
        }


    protected:
        const utl::Json& m_json;
        std::optional<Error> m_error;
        std::list<std::function<void()>> m_assignments;


    private:
        template<JsonCompatible T>
        static Result<json_iterator> _check(const utl::Json::object_type& object, std::string_view name) {
            auto it = object.find(name);

            if (it == object.end()) {
                return make_error(
                    Severity::Warning,
                    "parse_error",
                    "Json doesn't have field with name \"{}\"",
                    name
                );
            }

            if (!it->second.is<T>()) {
                return make_error(
                    Severity::Warning,
                    "parse_error",
                    "Field with name \"{}\" has mismatched type \"{}\"",
                    name, meta::type_name_v<T>
                );
            }

            return it;
        }
    };
}

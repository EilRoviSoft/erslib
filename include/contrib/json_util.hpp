#pragma once

// std
#include <iostream>
#include <functional>
#include <list>

// utl
#include <contrib/json.hpp>

// ers
#include <erslib/type/result.hpp>


namespace ers::util {
    class Schema {
    public:
        explicit Schema(const utl::Json& json) :
            m_json(json) {
        }


        template<typename T>
        void require(std::string_view name) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); !r)
                m_error = std::move(r.error());
        }


        template<typename T>
        void write_if_exist(std::string_view name, T& out) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); r) {
                m_assignments.emplace_back([&out, value = (*r)->second] {
                    out = value.template as<T>();
                });
            }
        }


        template<typename T>
        void require_and_write(std::string_view name, T& out) {
            const auto& object = m_json.as_object();

            if (auto r = _check<T>(object, name); r) {
                m_assignments.emplace_back([&out, value = (*r)->second] {
                    out = value.template as<T>();
                });
            } else
                m_error = std::move(r.error());
        }

        template<typename T>
        void require_and_write(std::string_view name, T& out, std::function<Result<T>(std::string_view)> writer) {
            const auto& object = m_json.as_object();

            if (auto r = _check<utl::Json::string_type>(object, name); r) {
                m_assignments.emplace_back([&out, writer = std::move(writer), value = (*r)->second] {
                    if (auto r = writer(value.template as<utl::Json::string_type>()); r)
                        out = *r;
                    else
                        throw ers::Exception(r.error());
                });
            } else
                m_error = std::move(r.error());
        }


        Status finalize() const {
            if (m_error)
                return Unexpected(*m_error);

            for (const auto& assign : m_assignments)
                assign();

            return ok;
        }


    protected:
        const utl::Json& m_json;
        std::optional<Error> m_error;
        std::list<std::function<void()>> m_assignments;


    private:
        template<typename T>
        static Result<utl::Json::object_type::const_iterator> _check(const utl::Json::object_type& object, std::string_view name) {
            auto it = object.find(name);

            if (it == object.end()) {
                return Unexpected<Error>(
                    Severity::Warning,
                    "parse_error",
                    "Json doesn't have field with name \"{}\"",
                    name
                );
            }

            if (!it->second.is<T>()) {
                return Unexpected<Error>(
                    Severity::Warning,
                    "parse_error",
                    "Field with name \"{}\" has mismatched type \"{}\"",
                    name, ers::meta::type_name_v<T>
                );
            }

            return it;
        }
    };
}

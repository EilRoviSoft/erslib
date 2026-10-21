#pragma once

// std
#include <concepts>
#include <meta>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/flux/connection/scope.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/query/query_store.hpp>
#include <erslib/flux/reflect/bind.hpp>
#include <erslib/flux/reflect/row.hpp>
#include <erslib/flux/reflect/schema.hpp>
#include <erslib/flux/reflect/statement/traits.hpp>
#include <erslib/flux/result/row.hpp>


namespace flux::impl::reflect {
    template<Statement Tag>
    class QueryCall {
    public:
        // Member types

        using Input = input_t<Tag>;
        using Output = output_t<Tag>;
        using Fragments = fragments_t<Tag>;


        // Member functions

        QueryCall() requires(std::same_as<Input, void> && !HasFragments<Tag>) = default;

        explicit QueryCall(Fragments fragments) requires(std::same_as<Input, void> && HasFragments<Tag>) :
            _fragments(std::move(fragments)) {
        }

        template<std::same_as<Input> U = Input>
            requires (!std::is_void_v<U> && !HasFragments<Tag>)
        explicit QueryCall(U input) :
            _input(std::move(input)) {
        }

        template<std::same_as<Input> U = Input>
            requires (!std::is_void_v<U> && HasFragments<Tag>)
        QueryCall(U input, Fragments fragments) :
            _input(std::move(input)),
            _fragments(std::move(fragments)) {
        }


        // Executors

        QueryResult exec(pqxx::dbtransaction& tx) const {
            return QueryResult(_exec_raw(tx));
        }


    private:
        [[no_unique_address]]
        wrapped_input<Input> _input;
        
        [[no_unique_address]]
        Fragments _fragments;


        std::string_view _fragment_for(std::string_view name) const {
            std::string_view found;
            bool matched = false;

            static constexpr auto members = std::define_static_array(std::meta::nonstatic_data_members_of(
                ^^Fragments, std::meta::access_context::current()));
            
            template for (constexpr auto m : std::define_static_array(members)) {
                if (!matched && std::meta::identifier_of(m) == name) {
                    found = _fragments.[:m:];
                    matched = true;
                }
            }

            if (!matched) {
                throw ers::make_runtime_error("flux: query '{}' has no fragment named '{}'",
                    query_label<Tag>, name);
            }

            return found;
        }


        std::string _render(std::string_view sql) const {
            std::string out;
            out.reserve(sql.size());

            size_t i = 0;

            while (i < sql.size()) {
                auto open = sql.find("{{", i);
                if (open == std::string_view::npos) {
                    out += sql.substr(i);
                    break;
                }

                auto close = sql.find("}}", open + 2);
                if (close == std::string_view::npos) {
                    throw ers::make_runtime_error("flux: unterminated fragment marker in query '{}'",
                        query_label<Tag>);
                }

                out += sql.substr(i, open - i);
                out += _fragment_for(sql.substr(open + 2, close - open - 2));

                i = close + 2;
            }

            return out;
        }


        pqxx::result _exec_raw(pqxx::dbtransaction& tx) const {
            auto sql = queries.get(std::string(query_label<Tag>));
            if (!sql) {
                throw ers::make_runtime_error("flux: no query registered for label '{}'",
                    query_label<Tag>);
            }

            pqxx::params params;

            if constexpr (!std::same_as<Input, void>)
                bind_params(params, _input.value);

            if constexpr (HasFragments<Tag>)
                return tx.exec(_render(*sql), params);
            else
                return tx.exec(*sql, params);
        }
    };


    template<typename Tag>
    struct QueryFn {
        template<typename T = Tag>
            requires (std::is_void_v<input_t<T>> && !HasFragments<T>)
        auto operator()() const {
            return QueryCall<T>();
        }

        template<typename T = Tag>
            requires (std::is_void_v<input_t<T>> && HasFragments<T>)
        auto operator()(fragments_t<T> fragments) const {
            return QueryCall<T>(std::move(fragments));
        }

        template<typename T = Tag>
            requires (!std::is_void_v<input_t<T>> && !HasFragments<T>)
        auto operator()(input_t<T> input) const {
            return QueryCall<T>(std::move(input));
        }

        template<typename T = Tag>
            requires (!std::is_void_v<input_t<T>> && HasFragments<T>)
        auto operator()(input_t<T> input, fragments_t<T> fragments) const {
            return QueryCall<T>(std::move(input), std::move(fragments));
        }

        template<typename T = Tag, typename... Args>
            requires (!std::is_void_v<input_t<T>>
                && !HasFragments<T>
                && (sizeof...(Args) != 1 || !(std::same_as<std::remove_cvref_t<Args>, input_t<T>> && ...)))
        auto operator()(Args&&... args) const {
            return QueryCall<T>(input_t<T>(std::forward<Args>(args)...));
        }
    };

    template<typename Tag>
    inline constexpr QueryFn<Tag> query_fn {};
}

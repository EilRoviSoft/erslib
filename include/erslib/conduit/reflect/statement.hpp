#pragma once

// std
#include <meta>
#include <string>
#include <string_view>
#include <utility>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/query_builder.hpp>
#include <erslib/conduit/impl/query_store.hpp>
#include <erslib/conduit/impl/scope.hpp>
#include <erslib/conduit/reflect/bind.hpp>
#include <erslib/conduit/reflect/row.hpp>
#include <erslib/conduit/reflect/schema.hpp>


// Implementation

namespace conduit::impl::reflect {
    template<typename Tag>
    concept HasFragments = requires { typename Declaration<Tag>::Fragments; };

    struct no_fragments {};

    template<typename Tag>
    struct fragments_of {
        using type = no_fragments;
    };

    template<typename Tag>
        requires HasFragments<Tag>
    struct fragments_of<Tag> {
        using type = typename Declaration<Tag>::Fragments;
    };

    template<typename Tag>
    using fragments_t = typename fragments_of<Tag>::type;
}

namespace conduit::impl::reflect {
    template<typename Tag>
    concept HasInput = requires { typename Declaration<Tag>::Input; };

    template<typename Tag>
    struct input_of {
        using type = void;
    };

    template<typename Tag>
        requires HasInput<Tag>
    struct input_of<Tag> {
        using type = typename Declaration<Tag>::Input;
    };

    template<typename Tag>
    using input_t = typename input_of<Tag>::type;


    template<typename T>
    struct wrapped_input {
        T value;
    };

    template<>
    struct wrapped_input<void> {};
}

namespace conduit::impl::reflect {
    template<typename Tag>
    concept HasOutput = requires { typename Declaration<Tag>::Output; };

    template<typename Tag>
    struct output_of {
        using type = void;
    };

    template<typename Tag>
        requires HasOutput<Tag>
    struct output_of<Tag> {
        using type = typename Declaration<Tag>::Output;
    };

    template<typename Tag>
    using output_t = typename output_of<Tag>::type;
}


// QueryCall

namespace conduit::impl::reflect {
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
                throw ers::make_runtime_error("conduit: query '{}' has no fragment named '{}'",
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
                    throw ers::make_runtime_error("conduit: unterminated fragment marker in query '{}'",
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
                throw ers::make_runtime_error("conduit: no query registered for label '{}'",
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


// Shortcuts

#define ERS_DECLARE_QUERY(NS, NAME, LABEL) \
    namespace NS { \
        struct NAME##_tag {}; \
        inline constexpr auto NAME = conduit::reflect::query_fn<NAME##_tag>; \
    } \
    \
    template<> \
    struct conduit::reflect::Declaration<NS::NAME##_tag> : conduit::reflect::Query<LABEL>

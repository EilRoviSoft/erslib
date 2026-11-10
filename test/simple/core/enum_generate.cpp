// catch2
#include <catch2/catch_test_macros.hpp>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <meta>
#include <string>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/enum/generate.hpp>
#include <erslib/core/stdlib/ctype.hpp>

// test
#include "support/test_case.hpp"


namespace {
    enum Outcome {
        SKIPPED = 0,
        OK = 200,
        CREATED = 201,
        NOT_FOUND = 404,
        LOCKED = 423,
    };

    enum class Mode {
        Fast,
        Slow,
    };


    struct OutcomePolicy {
        using enum_type = Outcome;

        static consteval bool is_enabled(Outcome value, std::string_view) {
            return value != SKIPPED;
        }

        static consteval std::string get_name(Outcome, std::string_view id) {
            return "make_" + ers::to_lower(id);
        }

        template<Outcome Value>
        struct Entry {
            static constexpr Outcome value = Value;

            constexpr int operator()() const {
                return static_cast<int>(Value);
            }

            constexpr int operator()(int extra) const {
                return static_cast<int>(Value) + extra;
            }

            std::string operator()(std::string_view text) const
                requires (Value >= 400) {
                return std::string(text) + "!";
            }
        };

        template<Outcome Value>
        using entry = Entry<Value>;
    };

    struct ModePolicy {
        using enum_type = Mode;

        static consteval bool is_enabled(Mode, std::string_view) {
            return true;
        }

        static consteval std::string get_name(Mode, std::string_view id) {
            return std::string(id);
        }

        template<Mode Value>
        struct Entry {
            constexpr bool operator()() const {
                return Value == Mode::Slow;
            }
        };

        template<Mode Value>
        using entry = Entry<Value>;
    };

    struct NoNamePolicy {
        using enum_type = Mode;

        static consteval bool is_enabled(Mode, std::string_view) {
            return true;
        }
    };

    struct NotAnEnumPolicy {
        using enum_type = int;

        static consteval bool is_enabled(int, std::string_view) {
            return true;
        }

        static consteval std::string get_name(int, std::string_view) {
            return "x";
        }
    };


    inline constexpr auto outcomes = ers::enum_table<OutcomePolicy>;
    inline constexpr auto modes = ers::enum_table<ModePolicy>;


    template<typename T>
    consteval bool has_member(std::string_view name) {
        for (auto m : std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked())) {
            if (std::meta::identifier_of(m) == name)
                return true;
        }

        return false;
    }

    template<typename T, typename... Args>
    concept callable_with = requires(const T& entry, Args&&... args) {
        entry(std::forward<Args>(args)...);
    };

    template<typename T>
    using plain_t = std::remove_cvref_t<T>;
}


// Policy ----------------------------------------------------------------------

static_assert(ers::impl::enum_gen::Policy<OutcomePolicy>);
static_assert(ers::impl::enum_gen::Policy<ModePolicy>);
static_assert(!ers::impl::enum_gen::Policy<NoNamePolicy>);
static_assert(!ers::impl::enum_gen::Policy<NotAnEnumPolicy>);


// Names -----------------------------------------------------------------------

namespace gen = ers::impl::enum_gen;

static_assert(gen::is_identifier("make_ok"));
static_assert(gen::is_identifier("_x1"));
static_assert(gen::is_identifier("Fast"));
static_assert(!gen::is_identifier(""));
static_assert(!gen::is_identifier("1abc"));
static_assert(!gen::is_identifier("a-b"));
static_assert(!gen::is_identifier("a b"));

static_assert(gen::is_keyword("while"));
static_assert(gen::is_keyword("class"));
static_assert(gen::is_keyword("operator"));
static_assert(!gen::is_keyword("whilst"));
static_assert(!gen::is_keyword("make_while"));
static_assert(!gen::is_keyword(""));


// Members ---------------------------------------------------------------------

using outcome_table = ers::enum_table_t<OutcomePolicy>;
using mode_table = ers::enum_table_t<ModePolicy>;

static_assert(has_member<outcome_table>("make_ok"));
static_assert(has_member<outcome_table>("make_created"));
static_assert(has_member<outcome_table>("make_not_found"));
static_assert(has_member<outcome_table>("make_locked"));
static_assert(!has_member<outcome_table>("make_skipped"));
static_assert(!has_member<outcome_table>("make_conflict"));

static_assert(has_member<mode_table>("Fast"));
static_assert(has_member<mode_table>("Slow"));
static_assert(!has_member<mode_table>("make_fast"));

static_assert(std::is_same_v<plain_t<decltype(outcomes.make_ok)>, OutcomePolicy::Entry<OK>>);
static_assert(std::is_same_v<plain_t<decltype(outcomes.make_locked)>, OutcomePolicy::Entry<LOCKED>>);
static_assert(std::is_same_v<plain_t<decltype(modes.Slow)>, ModePolicy::Entry<Mode::Slow>>);

static_assert(!std::is_same_v<outcome_table, mode_table>);

static_assert(sizeof(outcomes) == 1);
static_assert(sizeof(modes) == 1);


// Overloads are constrained per value -----------------------------------------

using ok_entry = OutcomePolicy::Entry<OK>;
using not_found_entry = OutcomePolicy::Entry<NOT_FOUND>;

static_assert(callable_with<ok_entry>);
static_assert(callable_with<ok_entry, int>);
static_assert(!callable_with<ok_entry, std::string_view>);
static_assert(callable_with<not_found_entry, std::string_view>);


// Behaviour -------------------------------------------------------------------

ERS_TEST_CASE("enum generate: entries get their value", "[core]") {
    CHECK(outcomes.make_ok() == 200);
    CHECK(outcomes.make_created() == 201);
    CHECK(outcomes.make_not_found() == 404);
    CHECK(outcomes.make_locked() == 423);

    CHECK(outcomes.make_ok(5) == 205);
    CHECK(outcomes.make_locked(-23) == 400);
}

ERS_TEST_CASE("enum generate: constrained overloads", "[core]") {
    CHECK(outcomes.make_not_found(std::string_view("nope")) == "nope!");
    CHECK(outcomes.make_locked(std::string_view("busy")) == "busy!");
}

ERS_TEST_CASE("enum generate: constant evaluation", "[core]") {
    static_assert(outcomes.make_ok() == 200);
    static_assert(outcomes.make_created(1) == 202);
    static_assert(modes.Slow());
    static_assert(!modes.Fast());

    CHECK(modes.Slow());
    CHECK_FALSE(modes.Fast());
}

#endif

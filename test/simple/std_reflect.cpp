// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <format>
#include <meta>
#include <string>


constexpr std::array names {
    "item0", "item1", "item2", "item3",
    "item4", "item5", "item6", "item7",
    "item8", "item9", "item10", "item11",
    "item12", "item13", "item14", "item15",
};

struct A;
consteval {
std::meta::define_aggregate(^^A, {
    std::meta::data_member_spec(^^std::string, { .name = "x" }),
});
}

template<typename... Ts>
struct Tuple {
    struct Storage;

    consteval {
    std::vector<std::meta::info> member_specs;
    template for (int i {}; auto type : { ^^Ts... }) {
        member_specs.emplace_back(data_member_spec(type, {
            .name = names[i]
        }));
        ++i;
    }

    define_aggregate(^^Storage, member_specs);
    }

    Storage data;

    Tuple() : data {} {}
    Tuple(const Ts&... values) : data { values... } {}
};

TEST_CASE("Testing C++ reflection") {
    A a;
    a.x = "Hello";

    Tuple<char, char, char, int> t('C', '+', '+', 26);
}

#endif

// std
#include <ranges>
#include <string_view>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// ers
#include <erslib/splitting/regular.hpp>
#include <erslib/splitting/smart.hpp>

struct data_t {
    std::string_view input;
    std::vector<std::string_view> expected;
};

template<typename TSplit>
std::vector<std::string_view> process(std::string_view input) {
    return TSplit(input) | std::ranges::to<std::vector>();
}

template<typename... TArgs>
std::vector<std::string_view> make_vector(TArgs&&... args) {
    std::vector<std::string_view> result;

    result.reserve(sizeof...(args));
    (result.emplace_back(std::forward<TArgs>(args)), ...);

    return result;
}

TEST_CASE("regular", "[splitting]") {
    REQUIRE(process<ers::RegularSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::RegularSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::RegularSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "\"Sir", "Isaac", "Westcott\""));
}

TEST_CASE("smart", "[splitting]") {
    REQUIRE(process<ers::SmartSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::SmartSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::SmartSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "Sir Isaac Westcott"));
}

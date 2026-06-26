// doctest
#include <doctest/doctest.h>

// std
#include <ranges>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/splitting/regular.hpp>
#include <erslib/core/splitting/smart.hpp>


namespace {
    template<typename TSplit>
    std::vector<std::string_view> process(std::string_view input) {
        std::vector<std::string_view> result;

        for (auto it : TSplit(input))
            result.emplace_back(it);

        return result;
    }


    template<typename... TArgs>
    std::vector<std::string_view> make_vector(TArgs&&... args) {
        std::vector<std::string_view> result;

        result.reserve(sizeof...(args));
        (result.emplace_back(std::forward<TArgs>(args)), ...);

        return result;
    }
}


TEST_CASE("regular") {
    REQUIRE(process<ers::RegularSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::RegularSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::RegularSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "\"Sir", "Isaac", "Westcott\""));
}

TEST_CASE("smart") {
    REQUIRE(process<ers::SmartSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::SmartSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::SmartSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "Sir Isaac Westcott"));
}

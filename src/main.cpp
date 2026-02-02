// std
#include <algorithm>
#include <iostream>

// ers
#include <erslib/splitting/regular.hpp>
#include <erslib/splitting/smart.hpp>

namespace test::regular {
    std::string text = "hello world ";

    void c0() {
        for (auto [i, it] : ers::RegularSplitter(text) | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }

    void c1() {
        std::vector values(std::from_range, ers::RegularSplitter(text));
        for (auto [i, it] : values | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }
}

namespace test::smart {
    std::string text = R"(I love "Sir Isaac"  )";

    void c0() {
        for (auto [i, it] : ers::SmartSplitter(text) | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }

    void c1() {
        std::vector values(std::from_range, ers::SmartSplitter(text));
        for (auto [i, it] : values | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }
}

int main() {
    test::regular::c0();
    test::regular::c1();

    return 0;
}

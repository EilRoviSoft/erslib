// std
#include <algorithm>
#include <iostream>

// ers
#include <erslib/splitting/regular.hpp>

namespace {
    std::string text = "hello world ";

    void test0() {
        for (auto [i, it] : ers::RegularSplitter(text) | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }

    void test1() {
        std::vector values(std::from_range, ers::RegularSplitter(text));
        for (auto [i, it] : values | std::views::enumerate)
            std::cout << i << ':' << it << '\n';
    }
}

int main() {
    test0();
    test1();

    return 0;
}

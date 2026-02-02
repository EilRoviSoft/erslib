// gtest
#include <gtest/gtest.h>

// test cases
#include "error.hpp"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

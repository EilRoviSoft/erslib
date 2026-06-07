#pragma once


namespace ers {
    struct NonCopyable { // NOLINT(cppcoreguidelines-special-member-functions)
        NonCopyable() = default;

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;

        NonCopyable(NonCopyable&&) = delete;
        NonCopyable& operator=(NonCopyable&&) = delete;
    };


    struct MoveOnly { // NOLINT(cppcoreguidelines-special-member-functions)
        MoveOnly() = default;

        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
    };
}

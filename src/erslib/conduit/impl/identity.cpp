#include "erslib/conduit/impl/identity.hpp"


// Internal

namespace {
    bool is_head_char(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool is_tail_char(char c) {
        return is_head_char(c) || (c >= '0' && c <= '9') || c == '$';
    }
}


// Implementation

bool conduit::impl::is_identifier(std::string_view name) {
    if (name.empty())
        return false;

    bool expect_head = true;

    for (const char c : name) {
        if (c == '.') {
            if (expect_head)
                return false;

            expect_head = true;
            continue;
        }

        if (expect_head ? !is_head_char(c) : !is_tail_char(c))
            return false;

        expect_head = false;
    }

    return !expect_head;
}

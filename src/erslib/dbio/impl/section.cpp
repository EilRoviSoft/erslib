#include "erslib/dbio/impl/section.hpp"


// Internal

namespace {
    bool is_head_char(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool is_tail_char(char c) {
        return is_head_char(c) || (c >= '0' && c <= '9') || c == '$';
    }
}


dbio::internal::section_format_t dbio::internal::section_format(Section section) {
    switch (section) {
        case Section::Column:
            return { .prefix = "", .separator = ", " };
        case Section::Where:
            return { .prefix = "\nWHERE ", .separator = " AND " };
        case Section::OrderBy:
            return { .prefix = "\nORDER BY ", .separator = ", " };
        case Section::Limit:
            return { .prefix = "\nLIMIT ", .separator = " " };
        case Section::Offset:
            return { .prefix = "\nOFFSET ", .separator = " " };
        default:
            return { .prefix = "", .separator = " " };
    }
}

bool dbio::internal::is_singular(Section section) {
    return section == Section::Limit || section == Section::Offset;
}
bool dbio::internal::is_identifier(std::string_view name) {
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

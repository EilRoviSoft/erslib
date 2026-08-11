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


dbio::internal::section_format_t dbio::internal::section_format(Section sec) {
    switch (sec) {
        case section::select:
            return { .prefix = "", .separator = "", .suffix = "" };
        case section::insert_into:
            return { .prefix = "INSERT INTO ", .separator = "", .suffix = "" };
        case section::column:
            return { .prefix = "", .separator = ", ", .suffix = "" };
        case section::source:
            return { .prefix = "\nFROM ", .separator = "", .suffix = "" };
        case section::where:
            return { .prefix = "\nWHERE ", .separator = " AND ", .suffix = "" };
        case section::order_by:
            return { .prefix = "\nORDER BY ", .separator = ", ", .suffix = "" };
        case section::values:
            return { .prefix = "\nVALUES ", .separator = ", ", .suffix = "" };
        case section::limit:
            return { .prefix = "\nLIMIT ", .separator = " ", .suffix = "" };
        case section::offset:
            return { .prefix = "\nOFFSET ", .separator = " ", .suffix = "" };
        default:
            return { .prefix = "", .separator = " ", .suffix = "" };
    }
}

bool dbio::internal::is_singular(Section sec) {
    switch (sec) {
        case section::select:
        case section::insert_into:
        case section::source:
        case section::limit:
        case section::offset:
            return true;
        default:
            return false;
    }
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

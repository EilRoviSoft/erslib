#include "erslib/flux/migration/declaration_diff.hpp"

// std
#include <algorithm>
#include <format>
#include <ranges>

// ers
#include <erslib/core/formatter/range.hpp>


// Details ---------------------------------------------------------------------

namespace {
    std::vector<std::string_view> sorted_lines(std::string_view text) {
        std::vector<std::string_view> out;

        for (const auto line : std::views::split(text, '\n')) {
            if (!line.empty())
                out.emplace_back(line.begin(), line.end());
        }

        std::ranges::sort(out);
        return out;
    }


    void append_marked(std::vector<std::string>& out, char mark, std::span<const std::string_view> lines) {
        for (const auto it : lines) {
            out.push_back(std::format("{} {}",
                mark, it));
        }
    }


    std::string describe_removed(const flux::impl::declaration_mismatch_t& what) {
        static constexpr std::string_view fmt =
            "\n\t{0}: removed from code."
            "\n\t\tPut `-- flux: drop {0}` into an '.existing.sql' migration.";

        return std::format(fmt, what.table);
    }

    std::string describe_changed(const flux::impl::declaration_mismatch_t& what) {
        std::string out = std::format("\n\t{}:", what.table);

        if (what.recorded.empty()) {
            out += "\n\t\tRecorded declaration is unknown, a migration claimed another version of it.";
        } else {
            for (const auto& line : flux::impl::diff_declarations(what.recorded, what.declared->text))
                out += std::format("\n\t\t{}", line);
        }

        out += std::format("\n\t\tPut `-- flux: table {} {:016x}` into an '.existing.sql' migration.",
            what.table, what.declared->hash);

        return out;
    }

    std::string describe(const flux::impl::declaration_mismatch_t& what) {
        return what.declared ? describe_changed(what) : describe_removed(what);
    }
}


// Implementation --------------------------------------------------------------

std::vector<std::string> flux::impl::diff_declarations(std::string_view recorded, std::string_view declared) {
    auto before = sorted_lines(recorded);
    auto after = sorted_lines(declared);

    std::vector<std::string_view> removed, added;
    std::ranges::set_difference(before, after, std::back_inserter(removed));
    std::ranges::set_difference(after, before, std::back_inserter(added));

    std::vector<std::string> out;
    append_marked(out, '-', removed);
    append_marked(out, '+', added);

    return out;
}

std::string flux::impl::describe_mismatches(std::span<const declaration_mismatch_t> what) {
    auto details = what
        | std::views::transform(describe)
        | std::views::join
        | std::ranges::to<std::string>();

    return std::format("Declarations of {} table(s) differ from the database, a migration is required:{}",
        what.size(), details);
}

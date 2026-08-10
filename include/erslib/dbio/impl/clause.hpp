#pragma once

// pqxx
#include <pqxx/params>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/section.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    using ClausePtr = std::unique_ptr<class IClause>;

    class ERSLIB_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(Section section) : _section(section) {}

        virtual ~IClause() = default;


        // Accessors

        [[nodiscard]]
        Section section() const noexcept { return _section; }


        // Executors

        [[nodiscard]]
        virtual ers::Status render(build_context_t& ctx) const = 0;


        // Misc

        [[nodiscard]]
        virtual ClausePtr clone() const = 0;


    private:
        Section _section;
    };
}

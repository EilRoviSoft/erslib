#include "aescript/impl/field.hpp"

// std
#include <algorithm>

// aescript
#include <aescript/impl/context.hpp>


// Field

aescript::Field::Field(const Field& other) {
    _copy_from(other);
}
aescript::Field& aescript::Field::operator=(const Field& other) {
    _copy_from(other);
    return *this;
}


void aescript::Field::add(VerifierPtr ptr) {
    _verifiers.emplace_front(std::move(ptr));

    auto inorder_it = std::ranges::lower_bound(
        _verifiers_order,
        _verifiers.begin(),
        [](const storage_iterator& lhs, const storage_iterator& rhs) {
        return (*lhs)->precedence() < (*rhs)->precedence();
    });

    _verifiers_order.emplace(inorder_it, _verifiers.begin());
}
void aescript::Field::add(ParserPtr ptr) {
    _parsers.emplace_front(std::move(ptr));
}


ers::Status aescript::Field::verify(sol::table table, std::string_view field) const {
    for (const auto& prop : _verifiers_order) {
        verify_context ctx;

        if (auto s = (*prop)->exec(ctx, table, field); !s)
            return s;

        if (ctx.skip)
            break;
    }

    return ers::ok;
}

ers::Status aescript::Field::parse(sol::table table, std::string_view field, void* where) const {
    // At this point we have already verified table
    // so we can skip processing of every absent field.

    if (!table.get<std::optional<sol::object>>(field))
        return ers::ok;


    for (const auto& prop : _parsers) {
        parser_context ctx;

        if (auto s = prop->exec(ctx, table, field, where); !s)
            return s;
    }

    return ers::ok;
}


void aescript::Field::_copy_from(const Field& other) {
    for (const auto& it : other._verifiers)
        add(it->clone());

    for (const auto& it : other._parsers)
        add(it->clone());
}


// Operators

aescript::Field& aescript::operator|(Field& lhs, VerifierPtr rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
aescript::Field&& aescript::operator|(Field&& lhs, VerifierPtr rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

aescript::Field& aescript::operator|(Field& lhs, ParserPtr rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
aescript::Field&& aescript::operator|(Field&& lhs, ParserPtr rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

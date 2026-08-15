#include "erslib/aescript/impl/field.hpp"

// std
#include <algorithm>

// ers
#include <erslib/aescript/impl/context.hpp>


// Field

aescript::impl::Field::Field(std::string name) :
    _name(std::move(name)) {
}

aescript::impl::Field::Field(const Field& other) {
    _copy_from(other);
}
aescript::impl::Field& aescript::impl::Field::operator=(const Field& other) {
    _copy_from(other);
    return *this;
}


void aescript::impl::Field::add(Verifier verifier) {
    _verifiers.emplace_front(std::move(verifier));

    auto inorder_it = std::ranges::lower_bound(
        _verifiers_order,
        _verifiers.begin(),
        [](const storage_iterator& lhs, const storage_iterator& rhs) {
        return (*lhs)->precedence() < (*rhs)->precedence();
    });

    _verifiers_order.emplace(inorder_it, _verifiers.begin());
}
void aescript::impl::Field::add(Parser parser) {
    _parsers.emplace_front(std::move(parser));
}


ers::Status aescript::impl::Field::verify(sol::table table) const {
    for (const auto& prop : _verifiers_order) {
        verify_context ctx;

        if (auto s = (*prop)->exec(ctx, table, _name); !s)
            return s;

        if (ctx.skip)
            break;
    }

    return ers::ok;
}

ers::Status aescript::impl::Field::parse(sol::table table, void* where) const {
    // At this point we have already verified table
    // so we can skip processing of every absent field.

    if (!table.get<std::optional<sol::object>>(_name))
        return ers::ok;


    for (const auto& prop : _parsers) {
        parser_context ctx;

        if (auto s = prop->exec(ctx, table, _name, where); !s)
            return s;
    }

    return ers::ok;
}


void aescript::impl::Field::_copy_from(const Field& other) {
    _name = other._name;

    _verifiers.clear();
    _verifiers_order.clear();
    _parsers.clear();

    for (const auto& it : other._verifiers)
        add(it);

    for (const auto& it : other._parsers)
        add(it);
}


// Operators

aescript::impl::Field& aescript::impl::operator|(Field& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
aescript::impl::Field&& aescript::impl::operator|(Field&& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

aescript::impl::Field& aescript::impl::operator|(Field& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
aescript::impl::Field&& aescript::impl::operator|(Field&& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

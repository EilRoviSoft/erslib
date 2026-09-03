#include "erslib/luaschema/impl/field.hpp"

// std
#include <algorithm>

// ers
#include <erslib/luaschema/impl/context.hpp>


// Field

ers::luaschema::impl::Field::Field(std::string name) :
    _name(std::move(name)) {
}

ers::luaschema::impl::Field::Field(const Field& other) {
    _copy_from(other);
}
ers::luaschema::impl::Field& ers::luaschema::impl::Field::operator=(const Field& other) {
    _copy_from(other);
    return *this;
}


void ers::luaschema::impl::Field::add(Verifier verifier) {
    _verifiers.emplace_front(std::move(verifier));

    auto inorder_it = std::ranges::lower_bound(
        _verifiers_order,
        _verifiers.begin(),
        [](const storage_iterator& lhs, const storage_iterator& rhs) {
        return (*lhs)->precedence() < (*rhs)->precedence();
    });

    _verifiers_order.emplace(inorder_it, _verifiers.begin());
}
void ers::luaschema::impl::Field::add(Parser parser) {
    _parsers.emplace_front(std::move(parser));
}


ers::Status ers::luaschema::impl::Field::verify(sol::table table) const {
    for (const auto& prop : _verifiers_order) {
        verify_context ctx;

        if (auto s = (*prop)->exec(ctx, table, _name); !s)
            return s;

        if (ctx.skip)
            break;
    }

    return ers::ok;
}

ers::Status ers::luaschema::impl::Field::parse(sol::table table, void* where) const {
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


void ers::luaschema::impl::Field::_copy_from(const Field& other) {
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

ers::luaschema::impl::Field& ers::luaschema::impl::operator|(Field& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
ers::luaschema::impl::Field&& ers::luaschema::impl::operator|(Field&& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

ers::luaschema::impl::Field& ers::luaschema::impl::operator|(Field& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
ers::luaschema::impl::Field&& ers::luaschema::impl::operator|(Field&& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

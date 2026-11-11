#include "erslib/luaschema/impl/field.hpp"

// std
#include <algorithm>
#include <optional>

// ers
#include <erslib/luaschema/impl/context.hpp>


// Implementation --------------------------------------------------------------

ers::impl::luaschema::Field::Field(std::string name) :
    _name(std::move(name)) {
}

ers::impl::luaschema::Field::Field(const Field& other) {
    _copy_from(other);
}
ers::impl::luaschema::Field& ers::impl::luaschema::Field::operator=(const Field& other) {
    _copy_from(other);
    return *this;
}


void ers::impl::luaschema::Field::add(Verifier verifier) {
    _verifiers.push_front(std::move(verifier));

    auto inorder_it = std::ranges::lower_bound(
        _verifiers_order,
        _verifiers.begin(),
        [](const storage_iterator& lhs, const storage_iterator& rhs) {
        return (*lhs)->precedence() < (*rhs)->precedence();
    });

    _verifiers_order.emplace(inorder_it, _verifiers.begin());
}
void ers::impl::luaschema::Field::add(Parser parser) {
    _parsers.push_front(std::move(parser));
}


ers::Status ers::impl::luaschema::Field::verify(sol::table table) const {
    for (const auto& prop : _verifiers_order) {
        verify_context ctx;

        if (auto s = (*prop)->exec(ctx, table, _name); !s)
            return s;

        if (ctx.skip)
            break;
    }

    return ers::ok;
}

ers::Status ers::impl::luaschema::Field::parse(sol::table table, void* where) const {
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


void ers::impl::luaschema::Field::_copy_from(const Field& other) {
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

ers::impl::luaschema::Field& ers::impl::luaschema::operator|(Field& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
ers::impl::luaschema::Field&& ers::impl::luaschema::operator|(Field&& lhs, Verifier rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

ers::impl::luaschema::Field& ers::impl::luaschema::operator|(Field& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
ers::impl::luaschema::Field&& ers::impl::luaschema::operator|(Field&& lhs, Parser rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

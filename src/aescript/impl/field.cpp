#include "aescript/impl/field.hpp"

// std
#include <algorithm>

// aescript
#include <aescript/impl/property_context.hpp>


aescript::Field::Field(const Field& other) {
    _copy_from(other);
}
aescript::Field& aescript::Field::operator=(const Field& other) {
    _copy_from(other);
    return *this;
}


void aescript::Field::add(FieldPropertyPtr ptr) {
    _storage.emplace_front(std::move(ptr));

    auto inorder_it = std::ranges::lower_bound(_inorder, _storage.begin(), [](const storage_iterator& lhs, const storage_iterator& rhs) {
        return (*lhs)->precedence() < (*rhs)->precedence();
    });

    _inorder.emplace(inorder_it, _storage.begin());
}

ers::Status aescript::Field::verify(sol::table table, std::string_view field) const {
    for (const auto& prop : _inorder) {
        property_context ctx;

        if (auto s = (*prop)->verify(ctx, table, field); !s)
            return s;

        if (ctx.skip_checks)
            break;
    }

    return ers::ok;
}


void aescript::Field::_copy_from(const Field& other) {
    for (const auto& it : other._storage)
        add(it->clone());
}


aescript::Field& aescript::operator|(Field& lhs, FieldPropertyPtr rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}

aescript::Field&& aescript::operator|(Field&& lhs, FieldPropertyPtr rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

#include "erslib/flux/sql/context.hpp"


flux::impl::Context::Context(const pqxx::dbtransaction& tx) :
    params(tx) {
}

std::string flux::impl::Context::bind(const binder_t& binder) {
    if (binder)
        binder(params);
    else
        params.append();

    auto result = counter.get();
    counter.next();

    return result;
}

std::string flux::impl::Context::bind_once(const void* key, const binder_t& binder) {
    if (auto it = bound.find(key); it != bound.end())
        return it->second;

    auto result = bind(binder);
    bound.emplace(key, result);

    return result;
}

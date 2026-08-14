#include "erslib/dbio/impl/context.hpp"


dbio::impl::Context::Context(const pqxx::dbtransaction& tx) :
    params(tx) {
}

std::string dbio::impl::Context::bind(const binder_t& binder) {
    binder(params);

    auto result = counter.get();
    counter.next();

    return result;
}
std::string dbio::impl::Context::bind_null() {
    params.append();

    auto result = counter.get();
    counter.next();

    return result;
}

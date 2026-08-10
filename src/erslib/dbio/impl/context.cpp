#include "erslib/dbio/impl/context.hpp"


dbio::build_context_t::build_context_t(const pqxx::dbtransaction& tx) :
    params(tx) {
}

std::string dbio::build_context_t::bind(const internal::binder_t& binder) {
    binder(params);

    auto result = counter.get();
    counter.next();

    return result;
}
std::string dbio::build_context_t::bind_null() {
    params.append();

    auto result = counter.get();
    counter.next();

    return result;
}

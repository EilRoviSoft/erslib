#include "erslib/conduit/impl/context.hpp"


conduit::impl::Context::Context(const pqxx::dbtransaction& tx) :
    params(tx) {
}

std::string conduit::impl::Context::bind(const binder_t& binder) {
    if (binder)
        binder(params);
    else
        params.append();

    auto result = counter.get();
    counter.next();

    return result;
}

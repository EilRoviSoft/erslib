// std
#include <iostream>

// ers
#include <erslib/dbio/query.hpp>


namespace {
    auto select_users_by_id_and_name() {
        using namespace dbio::clauses;
        return select_from("users")
            | columns("id", "name");
    }

    auto select_users_and_audit_by_id() {
        using namespace dbio::clauses;
        return select_from("users")
            | column("id")
            | from("audit");
    }
}


int main() {
    const dbio::Query queries[] = {
        select_users_by_id_and_name(),
        select_users_and_audit_by_id(),
    };

    for (const dbio::Query& it : queries) {
        auto r = it.to_sql();
        std::cout << (r ? *r : "No result") << '\n';
    }

    return 0;
}

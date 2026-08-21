// std
#include <iostream>

// ers
#include <erslib/dbio/query_builder.hpp>


namespace {
    auto select_users_by_id_and_name() {
        ERS_QUICK_DBIO_USING;
        return select_from("users")
            | columns("id", "name");
    }

    auto insert_id_and_name_into_users() {
        ERS_QUICK_DBIO_USING;

        size_t id = 42;
        std::string name = "Bob";
        
        return insert_into("users")
            | columns("id", "name")
            | values(id, name);
    }
}


int main() {
    const dbio::QueryBuilder queries[] = {
        select_users_by_id_and_name(),
        insert_id_and_name_into_users(),
    };

    for (const dbio::QueryBuilder& it : queries) {
        auto r = it.to_sql();
        std::cout << (r ? *r : "No result") << '\n';
    }

    return 0;
}

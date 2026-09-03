// std
#include <iostream>

// ers
#include <erslib/conduit/query_builder.hpp>


namespace {
    auto select_users_by_id_and_name() {
        using namespace conduit::orm;
        return select_from("users")
            | columns("id", "name");
    }

    auto insert_id_and_name_into_users() {
        using namespace conduit::orm;

        size_t id = 42;
        std::string name = "Bob";
        
        return insert_into("users")
            | columns("id", "name")
            | values(id, name);
    }
}


int main() {
    const conduit::QueryBuilder queries[] = {
        select_users_by_id_and_name(),
        insert_id_and_name_into_users(),
    };

    for (const conduit::QueryBuilder& it : queries) {
        auto r = it.to_sql();
        std::cout << (r ? *r : "No result") << '\n';
    }

    return 0;
}

#include "erslib/dbio/query_store.hpp"

// std
#include <fstream>
#include <mutex>
#include <sstream>


namespace fs = std::filesystem;


namespace {
    std::string read_file(const fs::path& path) {
        std::ifstream stream(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    // "user/save" -> "sql.user.save"
    std::string make_label(const fs::path& relative) {
        std::string result = "sql";

        for (const auto& part : relative.parent_path())
            result += '.' + part.generic_string();

        result += '.' + relative.stem().generic_string();
        return result;
    }
}


void dbio::QueryStore::add(std::string_view label, std::string_view query) {
    std::unique_lock lock(m_mutex);
    m_data.emplace(label, query);
}

size_t dbio::QueryStore::load_directory(const fs::path& root) {
    if (!fs::is_directory(root))
        return 0;

    size_t count = 0;

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".sql")
            continue;

        const fs::path relative = fs::relative(entry.path(), root);
        add(make_label(relative), read_file(entry.path()));
        count++;
    }

    return count;
}

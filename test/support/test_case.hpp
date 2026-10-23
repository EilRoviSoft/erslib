#pragma once

// std
#include <string>
#include <string_view>
#include <unordered_set>


#ifndef TEST_TAGS
#define TEST_TAGS ""
#endif


namespace test {
    inline const char* reduce_name(std::string_view name) {
        std::string reduced;
        bool pending = false;

        for (char c : name) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
                if (pending && !reduced.empty())
                    reduced += '_';
                pending = false;
                reduced += c;
            } else
                pending = true;
        }

        static auto& pool = *new std::unordered_set<std::string>;
        return pool.insert(std::move(reduced)).first->c_str();
    }
}


#define ERS_TEST_CASE(NAME, TAGS) \
    TEST_CASE(test::reduce_name(NAME), TEST_TAGS TAGS)

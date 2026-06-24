#pragma once

// std
#include <functional>

// pqxx
#include <pqxx/row>


namespace dbio {
    // Sequential reader over a pqxx::row.
    // Keeps an internal cursor so generated entity code can pull fields one after another in declaration order.
    // Holds a reference to the wrapped row, so it can only live as long as the row.
    class PqxxRow {
    public:
        using size_type = pqxx::row::size_type;


        // Member functions

        explicit PqxxRow(const pqxx::row& r) :
            m_row(&r) {
        }


        // Accessors

        void skip(size_type diff = 1) {
            m_index += diff;
        }

        [[nodiscard]]
        bool has_next(size_type amount = 1) const {
            return m_index + amount <= m_row->size();
        }

        template<typename T = std::string_view>
        T next(size_type offset = 1) {
            T result = get<T>();
            m_index += offset;
            return result;
        }

        // Stores the field at the cursor into dest and advances.
        // Returns false if the cursor is past the end or the field can't be cast to T.
        template<typename T>
        bool store(T& dest) try {
            if (!has_next())
                return false;

            dest = get<T>();
            m_index++;

            return true;
        } catch (const pqxx::conversion_error&) {
            return false;
        }

        bool store(const std::function<void(PqxxRow&)>& func, size_type amount = 1) try {
            if (!has_next(amount))
                return false;

            func(*this);
            m_index += amount;

            return true;
        } catch (const pqxx::conversion_error&) {
            return false;
        }

        template<typename T>
        T seek() {
            return get<T>();
        }


    protected:
        size_type m_index = 0;
        const pqxx::row* m_row;

        template<typename T>
        T get() {
            return (*m_row)[m_index].template as<T>();
        }
    };

    using PqxxRowStream = PqxxRow;
}

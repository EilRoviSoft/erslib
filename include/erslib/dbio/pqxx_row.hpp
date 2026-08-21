#pragma once

// std
#include <functional>

// pqxx
#include <pqxx/row>


namespace dbio::impl {
    // Sequential reader over a pqxx::row_ref.
    class PqxxRow {
    public:
        using size_type = pqxx::row_ref::size_type;


        // Member functions

        explicit PqxxRow(pqxx::row_ref row) :
            m_row(row) {
        }


        // Accessors

        void skip(size_type diff = 1) {
            m_index += diff;
        }

        [[nodiscard]]
        bool has_next(size_type amount = 1) const {
            return m_index + amount <= m_row.size();
        }

        template<typename T = std::string_view>
        T next(size_type offset = 1) {
            T result = get<T>();
            m_index += offset;
            return result;
        }

        // Stores and advanced.
        // Returns false if error occurs.
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
        pqxx::row_ref m_row;

        template<typename T>
        T get() {
            return m_row[m_index].template as<T>();
        }
    };

    using PqxxRowStream = PqxxRow;
}


// Exports

namespace dbio {
    using impl::PqxxRow;
    using impl::PqxxRowStream;
}

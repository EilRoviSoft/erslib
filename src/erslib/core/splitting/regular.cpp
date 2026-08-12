#include "erslib/core/splitting/regular.hpp"

// Forward declaration

namespace ers::impl::splitting {
    template class TIterator<RegularIterator>;
}


// Implementation

ers::impl::splitting::RegularIterator::RegularIterator(const Processor<RegularIterator>& parent, size_t offset) :
    TIterator(parent, offset) {
}

void ers::impl::splitting::RegularIterator::_advance() {
    if (!m_parent || m_offset >= m_parent->m_storage.size()) {
        m_length = 0;
        return;
    }


    if (m_parent->m_delims.contains(m_parent->m_storage[m_offset]))
        m_offset++;


    m_length = 0;
    while (m_offset + m_length < m_parent->m_storage.size() && !m_parent->m_delims.contains(m_parent->m_storage[m_offset + m_length]))
        m_length++;
}

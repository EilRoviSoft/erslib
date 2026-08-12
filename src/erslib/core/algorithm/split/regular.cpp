#include "erslib/core/algorithm/splitting/regular.hpp"

// Forward declaration

namespace ers::impl::algo {
    template class TSplitIterator<RegularSplitIterator>;
}


// Implementation

ers::impl::algo::RegularSplitIterator::RegularSplitIterator(const SplitProcessor<RegularSplitIterator>& parent, size_t offset) :
    TSplitIterator(parent, offset) {
}

void ers::impl::algo::RegularSplitIterator::_advance() {
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

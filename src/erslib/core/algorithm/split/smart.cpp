#include "erslib/core/algorithm/split/smart.hpp"

// Forward declaration

namespace ers::impl::algo {
    template class TSplitIterator<SmartSplitIterator>;
}


// Implementation

ers::impl::algo::SmartSplitIterator::SmartSplitIterator(const SplitProcessor<SmartSplitIterator>& parent, std::size_t offset) :
    TSplitIterator(parent, offset) {
}

void ers::impl::algo::SmartSplitIterator::_advance() {
    if (!m_parent || m_offset >= m_parent->m_storage.size()) {
        m_length = 0;
        return;
    }


    while (m_parent->m_delims.contains(m_parent->m_storage[m_offset]))
        m_offset++;


    m_length = 0;
    if (m_parent->m_storage[m_offset] == '"') {
        m_offset++;
        while (m_offset + m_length < m_parent->m_storage.size() && m_parent->m_storage[m_offset + m_length] != '"')
            m_length++;
    } else {
        while (m_offset + m_length < m_parent->m_storage.size() && !m_parent->m_delims.contains(m_parent->m_storage[m_offset + m_length]))
            m_length++;
    }
}

#include "erslib/splitting/smart.hpp"

// Forward declaration

namespace ers::splitting {
    template class TIterator<SmartIterator>;
}


// Implementation

ers::splitting::SmartIterator::SmartIterator(const Processor<SmartIterator>& parent, std::size_t offset) :
    TIterator(parent, offset) {
}

void ers::splitting::SmartIterator::_advance() {
    if (!m_parent || m_offset >= m_parent->m_content.size()) {
        m_length = 0;
        return;
    }

    while (m_parent->m_delims.contains(m_parent->m_content[m_offset]))
        m_offset++;

    m_length = 0;
    if (m_parent->m_content[m_offset] == '"') {
        m_offset++;
        while (m_offset + m_length < m_parent->m_content.size() && m_parent->m_content[m_offset + m_length] != '"')
            m_length++;
    } else {
        while (m_offset + m_length < m_parent->m_content.size() && !m_parent->m_delims.contains(m_parent->m_content[m_offset + m_length]))
            m_length++;
    }
}

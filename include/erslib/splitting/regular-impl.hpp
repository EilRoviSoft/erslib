#pragma once

#include "erslib/splitting/regular.hpp"

namespace ers::splitting {
    template class TIterator<RegularIterator>;
}

ers::splitting::RegularIterator::RegularIterator(const Processor<RegularIterator>& parent, size_t offset) :
    TIterator(parent, offset) {
}

void ers::splitting::RegularIterator::_advance() {
    if (!m_parent || m_offset >= m_parent->m_content.size()) {
        m_length = 0;
        return;
    }

    if (m_parent->m_delims.contains(m_parent->m_content[m_offset]))
        m_offset++;

    m_length = 0;
    while (m_offset + m_length < m_parent->m_content.size() && !m_parent->m_delims.contains(m_parent->m_content[m_offset + m_length]))
        m_length++;
}

#include "erslib/core/algorithm/split/smart.hpp"

// frozen
#include <frozen/unordered_set.h>


// Details ---------------------------------------------------------------------

namespace ers::impl::algo {
    template class TSplitIterator<SmartSplitIterator>;
}


// Implementation --------------------------------------------------------------

ers::impl::algo::SmartSplitIterator::SmartSplitIterator(const SplitProcessor<SmartSplitIterator>& parent, std::size_t offset) :
    TSplitIterator(parent, offset) {
}

void ers::impl::algo::SmartSplitIterator::_advance() {
    m_length = 0;

    if (!m_parent)
        return;

    const auto storage = m_parent->m_storage;

    m_offset = std::min(m_offset + _pending_skip, storage.size());
    _pending_skip = 0;

    while (m_offset < storage.size() && m_parent->m_delims.contains(storage[m_offset]))
        m_offset++;

    if (m_offset >= storage.size())
        return;

    if (char quote = storage[m_offset]; quote == '"' || quote == '\'') {
        m_offset++;

        while (m_offset + m_length < storage.size() && storage[m_offset + m_length] != quote)
            m_length++;

        _pending_skip = m_offset + m_length < storage.size() ? 1 : 0;
        return;
    }

    while (m_offset + m_length < storage.size() && !m_parent->m_delims.contains(storage[m_offset + m_length]))
        m_length++;
}

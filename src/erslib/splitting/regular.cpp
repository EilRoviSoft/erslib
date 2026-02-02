#include "erslib/splitting/regular.hpp"

// Regular

ers::splitting::Regular::Regular(std::string_view content, std::string_view delims) :
    _content(content),
    _delims(delims.begin(), delims.end()) {
}

ers::splitting::Regular::iterator ers::splitting::Regular::begin() const {
    return iterator(*this, 0);
}
ers::splitting::Regular::iterator ers::splitting::Regular::end() const {
    return iterator(*this, _content.size());
}

std::string_view ers::splitting::Regular::base() const {
    return _content;
}

// RegularIterator

ers::splitting::RegularIterator::RegularIterator(const Regular& parent, size_t offset) :
    _parent(&parent),
    _offset(offset) {
    if (_can_advance()) {
        _advance_to_next_token();
        _set_next_token_length();
    } else
        _length = 0;
}

ers::splitting::RegularIterator::value_type ers::splitting::RegularIterator::operator*() const {
    if (!_parent || _offset >= _parent->_content.size())
        return {};
    return _parent->_content.substr(_offset, _length);
}

ers::splitting::RegularIterator& ers::splitting::RegularIterator::operator++() {
    if (_can_advance()) {
        _offset += _length;
        _advance_to_next_token();
        _set_next_token_length();
    } else
        _length = 0;

    return *this;
}
ers::splitting::RegularIterator ers::splitting::RegularIterator::operator++(int) {
    auto temp = *this;
    ++*this;
    return temp;
}

bool ers::splitting::RegularIterator::operator==(const RegularIterator& other) const {
    return _parent == other._parent && _offset == other._offset;
}

bool ers::splitting::RegularIterator::_can_advance() const {
    return _parent && _offset < _parent->_content.size();
}

void ers::splitting::RegularIterator::_advance_to_next_token() {
    while (_offset < _parent->_content.size() && _parent->_delims.contains(_parent->_content[_offset]))
        _offset++;
}
void ers::splitting::RegularIterator::_set_next_token_length() {
    _length = 0;
    while (_offset + _length < _parent->_content.size() && !_parent->_delims.contains(_parent->_content[_offset + _length]))
        _length++;
}

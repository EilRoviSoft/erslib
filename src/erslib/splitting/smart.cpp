#include "erslib/splitting/smart.hpp"

// Smart

ers::splitting::Smart::Smart(std::string_view content, std::string_view delims) :
    _content(content),
    _delims(delims.begin(), delims.end()) {
}

ers::splitting::Smart::iterator ers::splitting::Smart::begin() const {
    return iterator(*this, 0);
}
ers::splitting::Smart::iterator ers::splitting::Smart::end() const {
    return iterator(*this, _content.size());
}

std::string_view ers::splitting::Smart::base() const {
    return _content;
}

// SmartIterator

ers::splitting::SmartIterator::SmartIterator(const Smart& parent, std::size_t offset) :
    _parent(&parent),
    _offset(offset) {
    _advance();
}

ers::splitting::SmartIterator::value_type ers::splitting::SmartIterator::operator*() const {
    if (!_parent || _offset >= _parent->_content.size())
        return {};
    return _parent->_content.substr(_offset, _length);
}

ers::splitting::SmartIterator& ers::splitting::SmartIterator::operator++() {
    if (_parent && _offset < _parent->_content.size()) {
        _offset += _length;
        _advance();
    }

    return *this;
}
ers::splitting::SmartIterator ers::splitting::SmartIterator::operator++(int) {
    auto temp = *this;
    ++*this;
    return temp;
}

bool ers::splitting::SmartIterator::operator==(const SmartIterator& other) const {
    return _parent == other._parent && _offset == other._offset;
}

void ers::splitting::SmartIterator::_advance() {
    if (!_parent || _offset >= _parent->_content.size()) {
        _length = 0;
        return;
    }

    while (_parent->_delims.contains(_parent->_content[_offset]))
        _offset++;

    _length = 0;
    if (_parent->_content[_offset] == '"') {
        _offset++;
        while (_offset + _length < _parent->_content.size() && _parent->_content[_offset + _length] != '"')
            _length++;
    } else {
        while (_offset + _length < _parent->_content.size() && !_parent->_delims.contains(_parent->_content[_offset + _length]))
            _length++;
    }
}

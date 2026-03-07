#pragma once


namespace ers::pattern {
    template<typename T>
    class TProxy {
    public:
        TProxy(T& value) :
            _value(&value) {
        }

        TProxy(const TProxy& another) {
            this->_value = another._value;
        }
        TProxy& operator=(const TProxy& another) = default;

        TProxy(TProxy&& another) noexcept {
            this->_value = another._value;
        }
        TProxy& operator=(TProxy&& another) noexcept {
            this->_value = another._value;
            return *this;
        }

        T& operator*() { return *this->get(); }
        T* operator->() { return this->get(); }

        const T& operator*() const { return *this->get(); }
        const T* operator->() const { return this->get(); }

        T* get() { return this->_value; }
        const T* get() const { return this->_value; }

    private:
        T* _value;
    };
}

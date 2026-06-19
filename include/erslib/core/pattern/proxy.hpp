#pragma once


namespace ers::pattern {
    template<typename T>
    class TProxy {
    public:
        // Constructor

        TProxy(T& value) :
            _value(&value) {
        }


        // Copy constructor

        TProxy(const TProxy& another) {
            this->_value = another._value;
        }
        TProxy& operator=(const TProxy& another) = default;


        // Move constructor

        TProxy(TProxy&& another) noexcept {
            this->_value = another._value;
        }
        TProxy& operator=(TProxy&& another) noexcept {
            this->_value = another._value;
            return *this;
        }


        // Accessors

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

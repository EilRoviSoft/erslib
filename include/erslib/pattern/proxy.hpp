#pragma once


namespace ers::pattern {
    template<typename T>
    class IProxy {
    public:
        IProxy(T& value) :
            _value(&value) {
        }

        IProxy(const IProxy& another) {
            this->_value = another._value;
        }
        IProxy& operator=(const IProxy& another) = default;

        IProxy(IProxy&& another) noexcept {
            this->_value = another._value;
        }
        IProxy& operator=(IProxy&& another) noexcept {
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

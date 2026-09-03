#pragma once

// std
#include <atomic>
#include <functional>
#include <mutex>

// ers
#include <erslib/core/memory/holder.hpp>
#include <erslib/core/meta/type_hash.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>


// Forward declaration

namespace ers::modkit::impl {
    template<typename T>
    class TResource;
}


// Control block

namespace ers::modkit::impl {
    template<typename T>
    struct control_block_t {
        using ctor_fn = std::function<ers::Status(control_block_t& cb)>;
        using dtor_fn = std::function<void(control_block_t& cb)>;


        control_block_t(ctor_fn ctor, dtor_fn dtor) :
            ctor(std::move(ctor)),
            dtor(std::move(dtor)) {
        }


        optional<T> value = nullopt;
        std::atomic<size_t> refs = 0;
        std::mutex mutex;
        ctor_fn ctor;
        dtor_fn dtor;
    };

    template<typename T>
    using control_block_ptr = Holder<control_block_t<T>>;
}


// Handle

namespace ers::modkit::impl {
    template<typename T>
    class Handle {
        friend TResource<T>;


    public:
        // Constructor

        Handle() = default;


        Handle(const Handle& other) :
            m_cb(other.m_cb) {
            _acquire();
        }
        Handle& operator=(const Handle& other) {
            m_cb = other.m_cb;
            _acquire();
            return *this;
        }


        Handle(Handle&& other) noexcept :
            m_cb(other.m_cb) {
            other.m_cb = nullptr;
        }
        Handle& operator=(Handle&& other) noexcept {
            m_cb = other.m_cb;
            other.m_cb = nullptr;
            return *this;
        }


        // Destructor

        ~Handle() {
            _release();
        }


        // Accessors

        const T* get() const { return m_cb->value.get(); }

        const T* operator->() const { return m_cb->value.get(); }
        const T& operator*() const { return *m_cb->value; }


    protected:
        control_block_t<T>* m_cb = nullptr;


    private:
        explicit Handle(control_block_t<T>& cb) :
            m_cb(&cb) {
            _acquire();
        }


        void _acquire() {
            m_cb->refs.fetch_add(1, std::memory_order_release);
        }

        void _release() {
            if (m_cb && m_cb->refs.fetch_sub(1, std::memory_order_release) == 1)
                m_cb->dtor(*m_cb);
        }
    };
}


// Control block helper functions

namespace ers::modkit::impl {
    template<typename T>
    void default_dtor(control_block_t<T>& cb) {
        std::scoped_lock lock(cb.mutex);
        cb.value.reset();
    }
}


// TResource

namespace ers::modkit::impl {
    template<typename T>
    class TResource {
    public:
        using value_type = T;
        using control_block_type = control_block_t<T>;


        // Constructor

        explicit TResource(control_block_type::ctor_fn ctor, control_block_type::dtor_fn dtor = default_dtor<T>) :
            m_cb(make_holder<control_block_type>(std::move(ctor), std::move(dtor))) {
        }


        // Observers

        [[nodiscard]]
        ers::Result<Handle<T>> view() const {
            if (!m_cb->refs.load(std::memory_order_acquire)) {
                if (auto s = m_cb->ctor(*m_cb); !s)
                    return s.error();
            }


            return Handle<T>(*m_cb);
        }


        [[nodiscard]]
        size_t id() const {
            return meta::type_hash_v<T>;
        }


    protected:
        Holder<control_block_type> m_cb;
    };
}


// Exports

namespace ers::modkit {
    using impl::Handle;
    using impl::TResource;
}

namespace ers::modkit {
    template<typename T>
    struct resource_type {
        using type = TResource<T>;
    };

    template<typename T>
    using resource_type_t = typename resource_type<T>::type;
}

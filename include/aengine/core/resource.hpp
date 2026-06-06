#pragma once

// std
#include <atomic>
#include <functional>
#include <mutex>

// ers
#include <erslib/memory/holder.hpp>
#include <erslib/meta/type_hash.hpp>
#include <erslib/type/result.hpp>


// Forward declaration

namespace aengine {
    template<typename T>
    class TResource;
}


// Control block

namespace aengine::internal {
    template<typename T>
    struct control_block_t {
        using ctor_fn = std::function<ers::Status(control_block_t& cb)>;
        using dtor_fn = std::function<void(control_block_t& cb)>;


        control_block_t(ctor_fn ctor, dtor_fn dtor) :
            ctor(std::move(ctor)),
            dtor(std::move(dtor)) {
        }


        boost::optional<T> value = boost::none;
        std::atomic<size_t> refs = 0;
        std::mutex mutex;
        ctor_fn ctor;
        dtor_fn dtor;
    };

    template<typename T>
    using control_block_ptr = ers::Holder<control_block_t<T>>;
}


// Handle

namespace aengine {
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
        internal::control_block_t<T>* m_cb = nullptr;


    private:
        explicit Handle(internal::control_block_t<T>& cb) :
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

namespace aengine::internal {
    template<typename T>
    void default_dtor(control_block_t<T>& cb) {
        std::scoped_lock lock(cb.mutex);
        cb.value.reset();
    }
}


// TResource

namespace aengine {
    template<typename T>
    class TResource {
    public:
        using value_type = T;
        using control_block_type = internal::control_block_t<T>;


        // Constructor

        explicit TResource(control_block_type::ctor_fn ctor, control_block_type::dtor_fn dtor = internal::default_dtor<T>) :
            m_cb(ers::make_holder<control_block_type>(std::move(ctor), std::move(dtor))) {
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
            return ers::meta::type_hash_v<T>;
        }


    protected:
        ers::Holder<control_block_type> m_cb;
    };
}


// TResource helper types

namespace aengine {
    template<typename T>
    struct resource_type {
        using type = TResource<T>;
    };

    template<typename T>
    using resource_type_t = typename resource_type<T>::type;
}

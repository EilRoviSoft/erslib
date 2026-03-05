#pragma once

// std
#include <atomic>
#include <memory>
#include <mutex>

// ers
#include <erslib/assert.hpp>
#include <erslib/type/result.hpp>


// Forward declaration

namespace ers {
    template<typename T>
    class Accessor;
}


// Control block

namespace ers {
    template<typename T>
    struct control_block_t {
        std::atomic<std::shared_ptr<T>> value;
        std::mutex mutex;
    };

    template<typename T>
    using control_block_ptr = std::shared_ptr<control_block_t<T>>;


    template<typename T>
    control_block_ptr<T> make_control_block() {
        return std::make_shared<control_block_t<T>>();
    }
}


// Resource

namespace ers {
    template<typename T>
    class IResource {
        friend class Accessor<T>;


    public:
        // Constructor

        IResource() :
            m_cb(make_control_block<T>()) {
        }


        // Destructor

        virtual ~IResource() = default;


        // Observers

        Result<Accessor<T>> get() const {
            auto ptr = m_cb->value.load(std::memory_order_acquire);

            if (!ptr) {
                std::lock_guard lock(m_cb->mutex);

                ptr = m_cb->value.load();

                if (!ptr) {
                    auto r = load();
                    if (!r)
                        return Unexpected(r.error());

                    ptr = r.value();
                    m_cb->value.store(ptr, std::memory_order_release);
                }
            }

            return Accessor<T>(m_cb, std::move(ptr));
        }


    protected:
        control_block_ptr<T> m_cb;


        [[nodiscard]]
        virtual Result<T> load() const = 0;
        virtual Status unload() const = 0;
    };


    template<typename T>
    class IResourceFromDisk : public IResource<T> {
    public:
        explicit IResourceFromDisk(std::string_view path) :
            m_path(path) {
        }


    protected:
        std::string m_path;
    };
}


// Accessor

namespace ers {
    template<typename T>
    class Accessor {
    public:
        // Constructor

        Accessor() = default;

        Accessor(control_block_ptr<T> cb, std::shared_ptr<T> ptr) :
            m_cb(std::move(cb)),
            m_ptr(std::move(ptr)) {
        }


        // Destructor

        ~Accessor() {
            if (!m_ptr)
                return;

            if (m_ptr.use_count() == 2) {
                std::shared_ptr<T> expected = m_ptr;
                m_cb->value.compare_exchange_strong(expected, nullptr, std::memory_order_seq_cst);
            }
        }


        // Accessors

        T* operator->() { return m_ptr.get(); }
        const T* operator->() const { return m_ptr.get(); }

        T& operator*() { return *m_ptr; }
        const T& operator*() const { return *m_ptr; }


    protected:
        control_block_ptr<T> m_cb;
        std::shared_ptr<T> m_ptr;
    };
}

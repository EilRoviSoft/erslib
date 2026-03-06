#pragma once

// std
#include <atomic>
#include <memory>
#include <mutex>

// ers
#include <erslib/type/result.hpp>


// Control block

namespace ers::thread_safe {
    template<typename T>
    struct resource_data_t {
        std::atomic<std::shared_ptr<T>> value;
        std::mutex mutex;
    };

    template<typename T>
    using resource_data_ptr = std::shared_ptr<resource_data_t<T>>;


    template<typename T>
    resource_data_ptr<T> make_resource_data() {
        return std::make_shared<resource_data_t<T>>();
    }
}


// Accessor

namespace ers::thread_safe {
    template<typename T>
    class Accessor {
    public:
        // Constructor

        Accessor() = default;

        Accessor(resource_data_ptr<T> cb, std::shared_ptr<T> ptr) :
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
        resource_data_ptr<T> m_cb;
        std::shared_ptr<T> m_ptr;
    };
}


// Resource

namespace ers::thread_safe {
    template<typename T>
    class IResource {
        friend class Accessor<T>;


    public:
        using value_type = T;
        using accessor_type = Accessor<T>;


        // Constructor

        IResource() :
            m_cb(make_resource_data<T>()) {
        }


        // Destructor

        virtual ~IResource() = default;


        // Observers

        Result<accessor_type> get() const {
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

            return accessor_type(m_cb, std::move(ptr));
        }


    protected:
        resource_data_ptr<T> m_cb;


        [[nodiscard]]
        virtual Result<T> load() const = 0;
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

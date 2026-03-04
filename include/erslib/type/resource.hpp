#pragma once

// std
#include <atomic>
#include <memory>

// boost
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// ers
#include <erslib/assert.hpp>
#include <erslib/type/result.hpp>


// Forward declaration

namespace ers {
    template<typename T>
    class Accessor;
}


// Resource

namespace ers {
    template<typename T>
    class IResource {
        friend class Accessor<T>;


    public:
        // Destructor

        virtual ~IResource() = default;


        // Observers

        Result<Accessor<T>> get() const {
            boost::upgrade_lock r_lock(_mutex);

            if (!m_value) {
                boost::upgrade_to_unique_lock w_lock(r_lock);

                if (auto s = load(); !s) {
                    m_value = nullptr;
                    return Unexpected(s.error());
                }
            }

            return Accessor<T>(*this);
        }


    protected:
        mutable std::unique_ptr<T> m_value;
        mutable std::atomic<size_t> m_refs = 0;


        virtual Status load() const = 0;
        virtual void unload() const {
            m_value.reset();
        }


    private:
        mutable boost::upgrade_mutex _mutex;
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
        // Constructors

        Accessor(const IResource<T>& source) :
            m_source(&source) {
            _acquire();
        }


        Accessor(const Accessor& other) :
            m_source(other.m_source) {
            _acquire();
        }
        Accessor& operator=(const Accessor& other) {
            if (this == &other)
                return *this;

            if (m_source)
                _release();

            m_source = other.m_source;
            _acquire();

            return *this;
        }


        Accessor(Accessor&& other) noexcept :
            m_source(other.m_source) {
            other.m_source = nullptr;
        }
        Accessor& operator=(Accessor&& other) noexcept {
            if (m_source)
                _release();
            
            m_source = other.m_source;
            other.m_source = nullptr;
            
            return *this;
        }


        // Destructor

        ~Accessor() {
            _release();
        }


        // Accessors

        T* operator->() { return _data(); }
        const T* operator->() const { return _data(); }

        T& operator*() { return *_data(); }
        const T& operator*() const { return *_data(); }


    protected:
        const IResource<T>* m_source = nullptr;


    private:
        T* _data() {
            ERS_ASSERT(m_source && m_source->m_value);
            return m_source->m_value;
        }
        const T* _data() const {
            ERS_ASSERT(m_source && m_source->m_value);
            return m_source->m_value;
        }


        void _acquire() {
            m_source->m_refs.fetch_add(1, std::memory_order_relaxed);
        }

        void _release() {
            if (m_source->m_refs.fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                boost::unique_lock lock(m_source->_mutex);
                m_source->unload();
            }

            m_source = nullptr;
        }
    };
}

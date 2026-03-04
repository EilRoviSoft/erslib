#pragma once

// std
#include <memory>

// boost
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// ers
#include <erslib/type/result.hpp>


namespace ers {
    template<typename T>
    class IResource {
    public:
        using storage_type = std::shared_ptr<T>;


        // Destructor

        virtual ~IResource() = default;


        // Observers

        Result<storage_type> get() const {
            boost::upgrade_lock r_lock(_mutex);

            if (!m_value) {
                boost::upgrade_to_unique_lock w_lock(r_lock);

                if (auto s = load(); !s) {
                    m_value = nullptr;
                    return Unexpected(s.error());
                }
            }

            return m_value;
        }


    protected:
        mutable storage_type m_value = nullptr;


        virtual Status load() const = 0;
        virtual Status unload() const {
            m_value.reset();
            return ok;
        }


    private:
        mutable boost::upgrade_mutex _mutex;


        storage_type _make_external_ptr() {
            auto self = this;

            return storage_type(
                m_value.get(),
                [self, inner = m_value](T*) mutable {
                    inner.reset();

                    boost::unique_lock r_lock(self->_mutex);

                    if (self->m_value && self->m_value.use_count() == 1) {
                        boost::upgrade_to_unique_lock w_lock(r_lock);

                        self->unload();
                    }
                }
            );
        }
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

#pragma once

// ers
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/thread_safe/map.hpp>
#include <erslib/core/type/error.hpp>
#include <erslib/core/type/result.hpp>

// aengine
#include <erslib/aengine/resource.hpp>
#include <erslib/aengine/fwd.hpp>


namespace aengine {
    class ResourceManager {
        using underlying_container_type = HashMap<
            std::string, Object,
            ers::string_hash_adaptor<ers::RapidHash>,
            ers::equal_adaptor<std::string>
        >;

        using container_type = ers::thread_safe::Map<underlying_container_type>;


    public:
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;


        // Constructor

        ResourceManager() = default;


        // Capacity

        [[nodiscard]]
        bool empty() const { return m_data.empty(); }

        [[nodiscard]]
        size_t size() const { return m_data.size(); }


        // I/O

        template<typename T, typename K, typename... Args>
        bool set(const K& k, Args&&... args) {
            return m_data.set(k, Object::make<resource_type_t<T>>(std::forward<Args>(args)...));
        }


        template<typename T, typename K>
        ers::Result<Handle<T>> get(const K& k) const {
            auto result = m_data.get(k);


            if (!result) {
                return ers::make_error(
                    ers::Severity::Error,
                    "element is not found",
                    std::format("Element with key {} is not found", k)
                );
            }


            return result->template get<resource_type_t<T>>().view();
        }


    protected:
        container_type m_data;
    };
}

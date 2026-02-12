#pragma once

// std
#include <cstddef>
#include <memory_resource>

// ers
#include <erslib/assert.hpp>
#include <erslib/concept/sbo.hpp>

#include "erslib/meta/type_hash.hpp"

// Forward declaration

namespace ers::internal {
	template<size_t Size, size_t Align>
	struct TAnyImpl;
}

// Details

namespace ers::internal {
	template<size_t Size, size_t Align>
	struct TAnyStorage {
		union {
			void* heap;
			alignas(Align) std::byte buffer[Size];
		};
	};

	template<size_t Align>
	struct TAnyStorage<0, Align> {
		void* heap;
	};

	template<size_t Size, size_t Align>
	struct TAnyVtable {
		void (*destroy)(TAnyImpl<Size, Align>& what);
		void (*dealloc)(TAnyImpl<Size, Align>& what);
		void (*copy)(TAnyImpl<Size, Align>& dst, const void* src);
		void (*move)(TAnyImpl<Size, Align>& dst, void* src);

		template<typename T>
		static void impl_destroy(TAnyImpl<Size, Align>& what);

		template<typename T>
		static void impl_dealloc(TAnyImpl<Size, Align>& what);

		template<typename T>
		static void impl_trivial_copy(TAnyImpl<Size, Align>& dst, const void* src);

		template<typename T>
		static void impl_copy(TAnyImpl<Size, Align>& dst, const void* src);

		template<typename T>
		static void impl_move(TAnyImpl<Size, Align>& dst, void* src);

		template<typename T>
		static constexpr auto make();

		template<typename T>
		static constexpr const auto& get();
	};
}

// Declaration

namespace ers::internal {
	template<size_t Size, size_t Align>
	struct TAnyImpl {
		// Aliases
		
		using storage_type = TAnyStorage<Size, Align>;
		using vtable_type = TAnyVtable<Size, Align>;

		// Members

		std::pmr::memory_resource* mr;
		vtable_type* vtable;
		size_t type;
		SboPolicy policy;
		storage_type storage;

		// Methods

		template<typename T>
		void resize_to_fit() {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				if (policy != SboPolicy::Empty)
					vtable->destroy(*this);
			}

			// It's guaranteed that "type" will be changed only after memory allocations.
			// We can assume that if types are the same, required size is the same too.

			if (type == meta::type_hash_v<T>)
				return;

			if (policy == SboPolicy::Dynamic)
				vtable->dealloc(*this);

			if constexpr (!is_sbo_applicable_v<T, Size, Align>)
				storage.heap = mr->allocate(sizeof(T), alignof(T));

			policy = is_sbo_applicable_v<T, Size, Align>;
		}

		// Setters

		template<typename T>
		constexpr void set_type() {
			type = meta::type_hash_v<T>;
		}

		// Accessors

		void* data() {
			switch (policy) {
				case SboPolicy::Dynamic:
					return storage.heap;

				case SboPolicy::Embedded:
					return storage.buffer;

				default:
					return nullptr;
			}
		}

		template<typename T>
		T* data_as() {
			if constexpr (is_sbo_applicable_v<T, Size, Align>)
				return reinterpret_cast<T*>(storage.buffer);
			else
				return static_cast<T*>(storage.heap);
		}
	};
}

// Implementation

namespace ers::internal {
	template<size_t Size, size_t Align>
	template<typename T>
	void TAnyVtable<Size, Align>::impl_destroy(TAnyImpl<Size, Align>& what) {
		ERS_ASSERT(!std::is_trivially_destructible_v<T>);

		if constexpr (is_sbo_applicable_v<T, Size, Align>) {
			ERS_ASSERT(what.m_policy == SboPolicy::Embedded);
			std::destroy_at(reinterpret_cast<T*>(what.m_storage.buffer));
		}
		else {
			ERS_ASSERT(what.m_policy == SboPolicy::Dynamic);
			std::destroy_at(static_cast<T*>(what.m_storage.heap));
		}
	}

	template<size_t Size, size_t Align>
	template<typename T>
	void TAnyVtable<Size, Align>::impl_dealloc(TAnyImpl<Size, Align>& what) {
		if constexpr (!is_sbo_applicable_v<T, Size, Align>)
			what.mr->deallocate(what.storage.heap, sizeof(T), alignof(T));
	}

	template<size_t Size, size_t Align>
	template<typename T>
	void TAnyVtable<Size, Align>::impl_trivial_copy(TAnyImpl<Size, Align>& dst, const void* src) {
		dst.template resize_to_fit<T>(dst);

		T* dst_ptr = dst.template data_as<T>();
		const T* src_ptr = static_cast<const T*>(src);

		std::memcpy(dst_ptr, src_ptr, sizeof(T));
	}

	template<size_t Size, size_t Align>
	template<typename T>
	void TAnyVtable<Size, Align>::impl_copy(TAnyImpl<Size, Align>& dst, const void* src) {
		dst.template resize_to_fit<T>(dst);

		T* dst_ptr = dst.template data_as<T>();
		const T* src_ptr = static_cast<const T*>(src);

		std::construct_at(dst_ptr, *src_ptr);
	}

	template<size_t Size, size_t Align>
	template<typename T>
	void TAnyVtable<Size, Align>::impl_move(TAnyImpl<Size, Align>& dst, void* src) {
		dst.template resize_to_fit<T>(dst);

		T* dst_ptr = dst.template data_as<T>();
		T* src_ptr = static_cast<T*>(src);

		std::construct_at(dst_ptr, std::move(*src_ptr));
	}

	template<size_t Size, size_t Align>
	template<typename T>
	constexpr auto TAnyVtable<Size, Align>::make() {
		TAnyVtable result;

		if constexpr (!std::is_trivially_destructible_v<T>)
			result.destroy = &impl_destroy<T>;
		else
			result.destroy = nullptr;

		result.dealloc = &impl_dealloc<T>;

		if constexpr (std::is_trivially_copyable_v<T>)
			result.copy = &impl_trivial_copy<T>;
		else
			result.copy = &impl_copy<T>;

		if constexpr (std::is_trivially_move_constructible_v<T>)
			result.move = &impl_trivial_copy<T>;
		else
			result.move = &impl_move<T>;

		return result;
	}

	template<size_t Size, size_t Align>
	template<typename T>
	constexpr const auto& TAnyVtable<Size, Align>::get() {
		static constexpr auto instance = make<T>();
		return instance;
	}
}

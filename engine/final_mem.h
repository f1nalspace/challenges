#pragma once

#include <allocators>
#include <new>

namespace fs {
	namespace mem {
		template<typename T>
		class StackAllocator {
		public:
			typedef T value_type;
			typedef value_type* pointer;
			typedef const value_type* const_pointer;
			typedef value_type& reference;
			typedef const value_type& const_reference;
			typedef std::size_t size_type;
			typedef std::ptrdiff_t difference_type;
		public:
			template<typename U>
			struct rebind {
				typedef StackAllocator<U> other;
			};

		public:
			inline explicit StackAllocator() {}
			inline ~StackAllocator() {}
			inline explicit StackAllocator(StackAllocator const&) {}
			template<typename U>
			inline explicit StackAllocator(StackAllocator<U> const&) {}

			inline pointer address(reference r) { return &r; }
			inline const_pointer address(const_reference r) { return &r; }

			inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0) {
				void *mem = alloca(cnt * sizeof(T));
				return new (mem)T;
			}
			inline void deallocate(pointer p, size_type) {
				// @NOTE: Stack allocated memory does not to be deallocated.
			}

			inline size_type max_size() const {
				return std::numeric_limits<size_type>::max() / sizeof(T);
			}

			inline void construct(pointer p, const T& t) { new(p) T(t); }
			inline void destroy(pointer p) { p->~T(); }

			inline bool operator==(StackAllocator const&) { return true; }
			inline bool operator!=(StackAllocator const& a) { return !operator==(a); }
		};
	};
};
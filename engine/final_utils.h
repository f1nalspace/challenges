#pragma once

#include <assert.h>

namespace fs {
	namespace utils {
		template <typename T>
		inline void Swap(T &a, T &b) {
			T tmp = a;
			a = b;
			b = tmp;
		}

		template <typename T, u64 N>
		inline u64 ArrayCount(T(&arr)[N]) {
			u64 result = sizeof(arr) / sizeof(arr[0]);
			return(result);
		}

		template <typename T>
		inline T PointerToValue(void *ptr) {
			T result = (T)(uintptr_t)ptr;
			return(result);
		}

		template <typename T>
		inline void *ValueToPointer(const T value) {
			void *result = (void *)(uintptr_t)value;
			return(result);
		}

		template <typename T, typename U, u64 N>
		inline void ArrayRemoveAndKeepOrder(T (&arr)[N], const U indexToRemove, U &count) {
			assert(indexToRemove < N);
			U oldListCount = count;
			assert(oldListCount > 0);
			for (U itemIndex = indexToRemove; itemIndex < oldListCount - 1; ++itemIndex) {
				T tmp = arr[itemIndex];
				arr[itemIndex] = arr[itemIndex + 1];
				arr[itemIndex + 1] = tmp;
			}
			count = oldListCount - 1;
		}

	};
};

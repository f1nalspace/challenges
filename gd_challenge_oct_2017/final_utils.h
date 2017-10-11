#pragma once

namespace finalspace {
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

	};
};

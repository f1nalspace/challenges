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
	};
};

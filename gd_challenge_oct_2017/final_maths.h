#pragma once

#include "final_types.h"

#include <cmath>

namespace finalspace {
	namespace maths {

		union Vec2i {
			struct {
				s32 x;
				s32 y;
			};
			struct {
				s32 w;
				s32 h;
			};
			s32 elements[2];

			inline Vec2i() {
				x = y = 0;
			}
			inline Vec2i(s32 value) {
				x = y = value;
			}
			inline Vec2i(s32 newX, s32 newY) {
				x = newX;
				y = newY;
			}
			inline Vec2i(const Vec2i &from) {
				x = from.x;
				y = from.y;
			}
		};

		union Vec2f {
			struct {
				f32 x;
				f32 y;
			};
			struct {
				f32 w;
				f32 h;
			};
			f32 elements[2];

#if 0
			inline Vec2f() {
				x = y = 0;
			}
			inline Vec2f(f32 value) {
				x = y = value;
			}
			inline Vec2f(f32 newX, f32 newY) {
				x = newX;
				y = newY;
			}
			inline Vec2f(const Vec2f &from) {
				x = from.x;
				y = from.y;
			}
#endif

			// @NOTE: Static initialization is done in the source file
			static const Vec2f Up;
			static const Vec2f Down;
			static const Vec2f Left;
			static const Vec2f Right;
		};

		template<typename T>
		inline T Clamp(const T value, const T min, const T max) {
			T result = value;
			if (result < min) result = min;
			if (result > max) result = max;
			return(result);
		}

		template<typename T>
		inline T Maximum(const T a, const T b) {
			T result;
			if (b > a) {
				result = b;
			} else {
				result = a;
			}
			return(result);
		}

		template<typename T>
		inline T Minimum(const T a, const T b) {
			T result;
			if (b < a) {
				result = b;
			} else {
				result = a;
			}
			return(result);
		}

		inline Vec2f operator + (const Vec2f &a, const Vec2f &b) {
			Vec2f result = Vec2f();
			result.x = a.x + b.x;
			result.y = a.y + b.y;
			return(result);
		}
		inline Vec2f &operator += (Vec2f &left, const Vec2f &right) {
			left.x += right.x;
			left.y += right.y;
			return(left);
		}
		inline Vec2f operator - (const Vec2f &a, const Vec2f &b) {
			Vec2f result = Vec2f();
			result.x = a.x - b.x;
			result.y = a.y - b.y;
			return(result);
		}
		inline Vec2f &operator -= (Vec2f &left, const Vec2f &right) {
			left.x -= right.x;
			left.y -= right.y;
			return(left);
		}
		inline Vec2f operator * (const f32 scalar, const Vec2f &vec) {
			Vec2f result = Vec2f();
			result.x = vec.x * scalar;
			result.y = vec.y * scalar;
			return(result);
		}
		inline Vec2f operator * (const Vec2f &vec, const f32 scalar) {
			Vec2f result = scalar * vec;
			return(result);
		}
		inline Vec2f &operator *= (Vec2f &left, const f32 right) {
			left.x *= right;
			left.y *= right;
			return(left);
		}
		inline Vec2f operator - (const Vec2f &v) {
			Vec2f result = { -v.x, -v.y };
			return(result);
		}

		inline f32 Dot(const Vec2f &a, const Vec2f &b) {
			f32 result = a.x * b.x + a.y * b.y;
			return(result);
		}

		inline f32 Length(const Vec2f &vec) {
			f32 result = (f32)sqrtf(vec.x * vec.x + vec.y * vec.y);
			return(result);
		}

		inline Vec2f Normalize(const Vec2f &vec) {
			f32 len = Length(vec);
			f32 invLen = len != 0.0f ? 1.0f / len : 1.0f;
			Vec2f result = invLen * vec;
			return(result);
		}

	};
};

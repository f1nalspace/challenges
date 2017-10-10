#pragma once

#include "final_types.h"

#include <math.h>

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
		};

		template<typename T>
		inline T Clamp(const T value, const T min, const T max) {
			T result = value;
			if (result < min) result = min;
			if (result > max) result = max;
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
		inline Vec2f &operator *= (Vec2f &left, const f32 right) {
			left.x *= right;
			left.y *= right;
			return(left);
		}

		inline f32 GetLength(const Vec2f &vec) {
			f32 result = (f32)sqrtf(vec.x * vec.x + vec.y * vec.y);
			return(result);
		}

		inline Vec2f Normalize(const Vec2f &vec) {
			f32 len = GetLength(vec);
			f32 invLen = len != 0.0f ? 1.0f / len : 1.0f;
			Vec2f result = invLen * vec;
			return(result);
		}

	};
};

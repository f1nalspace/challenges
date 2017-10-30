#pragma once

#include "final_types.h"

#include <assert.h>
#include <math.h>

#define MATH_ENABLE_SIMD 0

#if MATH_ENABLE_SIMD
#	include <xmmintrin.h>
#	include <intrin.h>
#endif

#define RGBA(r, g, b, a) ((u8)(a) << 24) | ((u8)(b) << 16) | ((u8)(g) << 8) | ((u8)(r) << 0)
#define RGBAToPixel(rgba) {((rgba) >> 0) & 0xFF, ((rgba) >> 8) & 0xFF, ((rgba) >> 16) & 0xFF, ((rgba) >> 24) & 0xFF}
#define PixelToRGBA(pixel) RGBAToPixel((pixel).r, (pixel).g, (pixel).b, (pixel).a)

namespace fs {
	namespace maths {
		//
		// Forward declarations
		//
		union Vec2f;
		Vec2f Cross(const f32 s, const Vec2f &a);
		Vec2f Cross(const Vec2f &a, const f32 s);

		//! Vec2i (2D signed 32-bit integer)
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

			inline Vec2i(const s32 xy = 0) {
				x = y = xy;
			}
			inline Vec2i(const s32 newX, const s32 newY) {
				x = newX;
				y = newY;
			}
			inline Vec2i(const Vec2i &from) {
				x = from.x;
				y = from.y;
			}
		};

		//
		// Vec2u (2D unsigned 32-bit integer)
		//
		union Vec2u {
			struct {
				u32 x;
				u32 y;
			};
			struct {
				u32 w;
				u32 h;
			};
			u32 elements[2];

			inline Vec2u(const u32 xy = 0) {
				x = y = xy;
			}
			inline Vec2u(const u32 newX, const u32 newY) {
				x = newX;
				y = newY;
			}
			inline Vec2u(const Vec2u &from) {
				x = from.x;
				y = from.y;
			}
		};

		//
		// Vec2f (2D 32-bit float)
		//
		union Vec2f {
			struct {
				f32 x;
				f32 y;
			};
			struct {
				f32 w;
				f32 h;
			};
			struct {
				f32 u;
				f32 v;
			};
			f32 elements[2];

			inline Vec2f(const f32 xy = 0.0f) {
				x = y = xy;
			}
			inline Vec2f(const f32 newX, const f32 newY) {
				x = newX;
				y = newY;
			}
			inline Vec2f(const Vec2f &from) {
				x = from.x;
				y = from.y;
			}

			inline Vec2f Swizzle(const u32 indexX = 0, const u32 indexY = 1) const {
				assert(indexX < 4);
				assert(indexY < 4);
				Vec2f result = Vec2f(elements[indexX], elements[indexY]);
				return(result);
			}

			// @NOTE: Static initialization is done in the source file
			static const Vec2f &Up;
			static const Vec2f &Down;
			static const Vec2f &Left;
			static const Vec2f &Right;
		};

		//
		// Vec3f (3D 32-bit float)
		//
		union Vec3f {
			struct {
				f32 x;
				f32 y;
				f32 z;
			};
			struct {
				f32 w;
				f32 h;
				f32 d;
			};
			struct {
				Vec2f xy;
				f32 z;
			};
			struct {
				f32 x;
				Vec2f yz;
			};
			struct {
				Vec2f st;
				f32 u;
			};
			struct {
				f32 s;
				Vec2f tu;
			};
			struct {
				f32 r;
				f32 g;
				f32 b;
			};
			struct {
				Vec2f rg;
				f32 b;
			};
			struct {
				f32 r;
				Vec2f gb;
			};
			f32 elements[3];

			inline Vec3f(const f32 xyz = 0.0f) {
				x = y = z = xyz;
			}
			inline Vec3f(const f32 newX, const f32 newY, const f32 newZ = 0.0f) {
				x = newX;
				y = newY;
				z = newZ;
			}
			inline Vec3f(const Vec2f &from, const f32 newZ = 0.0f) {
				x = from.x;
				y = from.y;
				z = newZ;
			}
			inline Vec3f(const Vec3f &from) {
				x = from.x;
				y = from.y;
				z = from.z;
			}

			inline Vec3f Swizzle(const u32 indexX = 0, const u32 indexY = 1, const u32 indexZ = 2) const {
				assert(indexX < 4);
				assert(indexY < 4);
				assert(indexZ < 4);
				Vec3f result = Vec3f(elements[indexX], elements[indexY], elements[indexZ]);
				return(result);
			}

			// @NOTE: Static initialization is done in the source file
			static const Vec3f &Up;
			static const Vec3f &Down;
			static const Vec3f &Left;
			static const Vec3f &Right;
		};

		//
		// Vec4f (4D 32-bit float)
		//
		union Vec4f {
			struct {
				f32 x;
				f32 y;
				f32 z;
				f32 w;
			};
			struct {
				Vec2f xy;
				f32 z;
				f32 w;
			};
			struct {
				f32 x;
				Vec2f yz;
				f32 w;
			};
			struct {
				f32 x;
				f32 y;
				Vec2f zw;
			};
			struct {
				Vec2f xy;
				Vec2f zw;
			};
			struct {
				Vec3f xyz;
				f32 w;
			};
			struct {
				f32 x;
				Vec3f yzw;
			};
			struct {
				f32 r;
				f32 g;
				f32 b;
				f32 a;
			};
			struct {
				Vec3f rgb;
				f32 a;
			};
			struct {
				f32 r;
				Vec3f gba;
			};
			struct {
				Vec2f rg;
				f32 b;
				f32 a;
			};
			struct {
				Vec2f rg;
				Vec2f ba;
			};
			struct {
				f32 r;
				Vec2f gb;
				f32 a;
			};
			struct {
				f32 r;
				f32 g;
				Vec2f ba;
			};
			f32 elements[4];

			inline Vec4f(const f32 newW = 1.0f) {
				x = y = z = 0;
				w = newW;
			}
			inline Vec4f(const f32 xyz, const f32 newW = 1.0f) {
				x = y = z = xyz;
				w = newW;
			}
			inline Vec4f(const f32 newX, const f32 newY, const f32 newZ, const f32 newW = 1.0f) {
				x = newX;
				y = newY;
				z = newZ;
				w = newW;
			}
			inline Vec4f(const Vec2f &from, const f32 newZ = 1.0f, const float newW = 1.0f) {
				x = from.x;
				y = from.y;
				z = newZ;
				w = newW;
			}
			inline Vec4f(const Vec3f &from, const float newW = 1.0f) {
				x = from.x;
				y = from.y;
				z = from.z;
				w = newW;
			}
			inline Vec4f(const Vec4f &from) {
				x = from.x;
				y = from.y;
				z = from.z;
				w = from.w;
			}

			inline Vec4f Swizzle(const u32 indexX = 0, const u32 indexY = 1, const u32 indexZ = 2, const u32 indexW = 3) const {
				assert(indexX < 4);
				assert(indexY < 4);
				assert(indexZ < 4);
				assert(indexW < 4);
				Vec4f result = Vec4f(elements[indexX], elements[indexY], elements[indexZ], elements[indexW]);
				return(result);
			}

			static const Vec4f &White;
			static const Vec4f &Black;
			static const Vec4f &Red;
			static const Vec4f &Green;
			static const Vec4f &Blue;
			static const Vec4f &Yellow;
		};

		//
		// Mat2f (2 x 2 float matrix)
		//
		union Mat2f {
			struct {
				Vec2f col1;
				Vec2f col2;
			};
			f32 m[4];

			Mat2f(const float d = 1.0f) {
				m[0] = d;
				m[1] = 0;
				m[2] = 0;
				m[3] = d;
			}

			Mat2f(const Mat2f &other) {
				m[0] = other.m[0];
				m[1] = other.m[1];
				m[2] = other.m[2];
				m[3] = other.m[3];
			}

			Mat2f(const f32 values[4]) {
				m[0] = values[0];
				m[1] = values[1];
				m[2] = values[2];
				m[3] = values[3];
			}

			Mat2f(const Vec2f &newCol1, const Vec2f &newCol2) {
				col1 = newCol1;
				col2 = newCol2;
			}

			static const Mat2f Identity;

			static Mat2f CreateRotation(const Vec2f &axis) {
				Mat2f result;
				result.col1 = axis;
				result.col2 = Cross(1.0f, axis);
				return(result);
			}

			static Mat2f CreateRotation(const float angle) {
				f32 s = sinf(angle);
				f32 c = cosf(angle);
				Mat2f result;
				result.col1 = Vec2f(c, s);
				result.col2 = Vec2f(-s, c);
				return(result);
			}
		};

		//
		// Mat4f (4 x 4 float matrix)
		//
		union Mat4f {
			struct {
				Vec4f col1;
				Vec4f col2;
				Vec4f col3;
				Vec4f col4;
			};
			struct {
				f32 elements[4][4];
			};
			f32 m[16];

			Mat4f(const float d = 1.0f) {
				m[0] = d;
				m[1] = 0;
				m[2] = 0;
				m[3] = 0;

				m[4] = 0;
				m[5] = d;
				m[6] = 0;
				m[7] = 0;

				m[8] = 0;
				m[9] = 0;
				m[10] = d;
				m[11] = 0;

				m[12] = 0;
				m[13] = 0;
				m[14] = 0;
				m[15] = d;
			}

			Mat4f(const Mat4f &other) {
				m[0] = other.m[0];
				m[1] = other.m[1];
				m[2] = other.m[2];
				m[3] = other.m[3];

				m[4] = other.m[4];
				m[5] = other.m[5];
				m[6] = other.m[6];
				m[7] = other.m[7];

				m[8] = other.m[8];
				m[9] = other.m[9];
				m[10] = other.m[10];
				m[11] = other.m[11];

				m[12] = other.m[12];
				m[13] = other.m[13];
				m[14] = other.m[14];
				m[15] = other.m[15];
			}

			Mat4f(const f32 values[16]) {
				m[0] = values[0];
				m[1] = values[1];
				m[2] = values[2];
				m[3] = values[3];

				m[4] = values[4];
				m[5] = values[5];
				m[6] = values[6];
				m[7] = values[7];

				m[8] = values[8];
				m[9] = values[9];
				m[10] = values[10];
				m[11] = values[11];

				m[12] = values[12];
				m[13] = values[13];
				m[14] = values[14];
				m[15] = values[15];
			}

			Mat4f(const Vec4f &newCol1, const Vec4f &newCol2, const Vec4f &newCol3, const Vec4f &newCol4) {
				col1 = newCol1;
				col2 = newCol2;
				col3 = newCol3;
				col4 = newCol4;
			}

			static inline Mat4f CreateRotation(const Mat2f &mat2) {
				Mat4f result = Mat4f(1.0f);
				result.col1.xy = mat2.col1;
				result.col2.xy = mat2.col2;
				return (result);
			}

			static inline Mat4f CreateTranslation(const Vec2f &p) {
				Mat4f result = Mat4f(1.0f);
				result.col4.x = p.x;
				result.col4.y = p.y;
				result.col4.z = 0.0f;
				return (result);
			}

			static inline Mat4f CreateTranslation(const Vec3f &p) {
				Mat4f result = Mat4f(1.0f);
				result.col4.x = p.x;
				result.col4.y = p.y;
				result.col4.z = p.z;
				return (result);
			}

			static inline Mat4f CreateScale(const Vec2f &s) {
				Mat4f result = Mat4f(1.0f);
				result.col1.x = s.x;
				result.col2.y = s.y;
				result.col3.z = 1.0f;
				return (result);
			}

			static inline Mat4f CreateScale(const Vec3f &s) {
				Mat4f result = Mat4f(1.0f);
				result.col1.x = s.x;
				result.col2.y = s.y;
				result.col3.z = s.z;
				return (result);
			}

			static inline Mat4f CreateOrtho(const f32 left, const f32 right, const f32 bottom, const f32 top) {
				Mat4f result = Mat4f(1);
				result.elements[0][0] = 2.0f / (right - left);
				result.elements[1][1] = 2.0f / (top - bottom);
				result.elements[2][2] = -1.0f;
				result.elements[3][0] = -(right + left) / (right - left);
				result.elements[3][1] = -(top + bottom) / (top - bottom);
				return(result);
			}

			static inline Mat4f CreateFrustumLH(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 zNear, const f32 zFar, const bool zeroToOneClipSpace = false) {
				Mat4f result = Mat4f(0.0f);
				result.elements[0][0] = (2.0f * zNear) / (right - left);
				result.elements[1][1] = (2.0f * zNear) / (top - bottom);
				result.elements[2][0] = (right + left) / (right - left);
				result.elements[2][1] = (top + bottom) / (top - bottom);
				result.elements[2][3] = 1.0f;
				if (zeroToOneClipSpace) {
					result.elements[2][2] = zFar / (zFar - zNear);
					result.elements[3][2] = -(zFar * zNear) / (zFar - zNear);
				} else {
					result.elements[2][2] = (zFar + zNear) / (zFar - zNear);
					result.elements[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
				}
				return(result);
			}

			static inline Mat4f CreateFrustumRH(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 zNear, const f32 zFar, const bool zeroToOneClipSpace = false) {
				Mat4f result = Mat4f(0.0f);
				result.elements[0][0] = (2.0f * zNear) / (right - left);
				result.elements[1][1] = (2.0f * zNear) / (top - bottom);
				result.elements[2][0] = (right + left) / (right - left);
				result.elements[2][1] = (top + bottom) / (top - bottom);
				result.elements[2][3] = -1.0f;
				if (zeroToOneClipSpace) {
					result.elements[2][2] = zFar / (zNear - zFar);
					result.elements[3][2] = -(zFar * zNear) / (zFar - zNear);
				} else {
					result.elements[2][2] = -(zFar + zNear) / (zFar - zNear);
					result.elements[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
				}
				return(result);
			}

			static inline Mat4f CreateOrthoLH(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 zNear, const f32 zFar, const bool zeroToOneClipSpace = false) {
				Mat4f result = Mat4f(1);
				result.elements[0][0] = 2.0f / (right - left);
				result.elements[1][1] = 2.0f / (top - bottom);
				result.elements[3][0] = -(right + left) / (right - left);
				result.elements[3][1] = -(top + bottom) / (top - bottom);
				if (zeroToOneClipSpace) {
					result.elements[2][2] = 1.0f / (zFar - zNear);
					result.elements[3][2] = -zNear / (zFar - zNear);
				} else {
					result.elements[2][2] = 2.0f / (zFar - zNear);
					result.elements[3][2] = -(zFar + zNear) / (zFar - zNear);
				}
				return(result);
			}

			static inline Mat4f CreateOrthoRH(const f32 left, const f32 right, const f32 bottom, const f32 top, const f32 zNear, const f32 zFar, const bool zeroToOneClipSpace = false) {
				Mat4f result = Mat4f(1);
				result.elements[0][0] = 2.0f / (right - left);
				result.elements[1][1] = 2.0f / (top - bottom);
				result.elements[3][0] = -(right + left) / (right - left);
				result.elements[3][1] = -(top + bottom) / (top - bottom);
				if (zeroToOneClipSpace) {
					result.elements[2][2] = -1.0f / (zFar - zNear);
					result.elements[3][2] = -zNear / (zFar - zNear);
				} else {
					result.elements[2][2] = -2.0f / (zFar - zNear);
					result.elements[3][2] = -(zFar + zNear) / (zFar - zNear);
				}
				return(result);
			}

			static const Mat4f &Identity;
		};

		union Pixel {
			struct {
				u8 r, g, b, a;
			};
			u8 m[4];
		};

		//
		// Scalar operations
		//
		template <typename T>
		inline T Clamp(const T value, const T min, const T max) {
			T result = value;
			if (result < min) result = min;
			if (result > max) result = max;
			return(result);
		}

		template <typename T>
		inline T Maximum(const T a, const T b) {
			T result;
			if (b > a) {
				result = b;
			} else {
				result = a;
			}
			return(result);
		}

		template <typename T>
		inline T Minimum(const T a, const T b) {
			T result;
			if (b < a) {
				result = b;
			} else {
				result = a;
			}
			return(result);
		}

		template <typename T>
		inline T Lerp(const T a, const T t, const T b) {
			T result = (1 - t) * a + t * b;
			return(result);
		}

		template <typename T>
		inline T Absolute(const T value) {
			T result;
			if (value < 0) {
				result = -value;
			} else {
				result = value;
			}
			return(result);
		}

		template <typename T>
		inline bool IsEqual(const T a, const T b, const T tolerance) {
			bool result = Absolute(a - b) < tolerance;
			return(result);
		}

		template <typename T>
		inline T SignZero(const T value) {
			T result = 0;
			if (value < 0) result = -1;
			else if (value > 0) result = 1;
			return(result);
		}

		//
		// Vec2i functions (Do not depend on the operators)
		//
		inline bool IsEqual(const Vec2i &a, const Vec2i &b) {
			bool result = (a.x == b.x) && (a.y == b.y);
			return(result);
		}

		inline s32 Dot(const Vec2i &a, const Vec2i &b) {
			s32 result = a.x * b.x + a.y * b.y;
			return(result);
		}

		//
		// Vec2i operators (+, -, *, += etc.)
		//
		inline Vec2i operator + (const Vec2i &a, const Vec2i &b) {
			Vec2i result = Vec2i();
			result.x = a.x + b.x;
			result.y = a.y + b.y;
			return(result);
		}
		inline Vec2i &operator += (Vec2i &left, const Vec2i &right) {
			left.x += right.x;
			left.y += right.y;
			return(left);
		}
		inline Vec2i operator - (const Vec2i &a, const Vec2i &b) {
			Vec2i result = Vec2i();
			result.x = a.x - b.x;
			result.y = a.y - b.y;
			return(result);
		}
		inline Vec2i &operator -= (Vec2i &left, const Vec2i &right) {
			left.x -= right.x;
			left.y -= right.y;
			return(left);
		}

		//
		// Vec2f functions (Do not depend on the operators)
		//
		inline Vec2f Absolute(const Vec2f &v) {
			Vec2f result = Vec2f(Absolute(v.x), Absolute(v.y));
			return(result);
		}

		inline bool IsEqual(const Vec2f &a, const Vec2f &b, const f32 tolerance) {
			bool result = IsEqual(a.x, b.x, tolerance) && IsEqual(a.y, b.y, tolerance);
			return(result);
		}

		inline f32 Dot(const Vec2f &a, const Vec2f &b) {
			f32 result = a.x * b.x + a.y * b.y;
			return(result);
		}

		inline f32 LengthSquared(const Vec2f &vec) {
			f32 result = vec.x * vec.x + vec.y * vec.y;
			return(result);
		}
		inline f32 Length(const Vec2f &vec) {
			f32 result = (f32)sqrtf(vec.x * vec.x + vec.y * vec.y);
			return(result);
		}

		inline f32 DistanceSquared(const Vec2f &a, const Vec2f &b) {
			f32 result = (a.x - b.x) * (a.y - b.y);
			return(result);
		}
		inline f32 Distance(const Vec2f &a, const Vec2f &b) {
			f32 result = (f32)sqrtf((a.x - b.x) * (a.y - b.y));
			return(result);
		}

		inline Vec2f Normalize(const Vec2f &vec) {
			f32 len = Length(vec);
			f32 invLen = len != 0.0f ? 1.0f / len : 1.0f;
			Vec2f result = Vec2f(vec.x * invLen, vec.y * invLen);
			return(result);
		}

		inline Vec2f Minimum(const Vec2f &a, const Vec2f &b) {
			Vec2f result;
			result.x = Minimum(a.x, b.x);
			result.y = Minimum(a.y, b.y);
			return(result);
		}

		inline Vec2f Maximum(const Vec2f &a, const Vec2f &b) {
			Vec2f result;
			result.x = Maximum(a.x, b.x);
			result.y = Maximum(a.y, b.y);
			return(result);
		}

		inline Vec2f Lerp(const Vec2f &a, const f32 t, const Vec2f &b) {
			Vec2f result;
			result.x = Lerp(a.x, t, b.x);
			result.y = Lerp(a.y, t, b.y);
			return(result);
		}

		inline Vec2f Clamp(const Vec2f &value, const Vec2f &min, const Vec2f &max) {
			Vec2f result;
			result.x = Clamp(value.x, min.x, max.x);
			result.y = Clamp(value.y, min.y, max.y);
			return(result);
		}

		inline Vec2f Hadamard(const Vec2f &a, const Vec2f &b) {
			Vec2f result = Vec2f(a.x * b.x, a.y * b.y);
			return(result);
		}

		inline Vec2f Cross(const f32 s, const Vec2f &a) {
			Vec2f result = Vec2f(-s * a.y, s * a.x);
			return(result);
		}

		inline Vec2f Cross(const Vec2f &a, const f32 s) {
			Vec2f result = Vec2f(s * a.y, -s * a.x);
			return(result);
		}

		inline f32 Cross(const Vec2f &a, const Vec2f &b) {
			f32 result = a.x * b.y - a.y * b.x;
			return(result);
		}

		//
		// Vec2f operators (+, -, *, += etc.)
		//
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

		inline Vec2f operator * (const Vec2f &a, const Mat2f &mat) {
			Vec2f result;
			result.x = mat.col1.x * a.x + mat.col2.x * a.y;
			result.y = mat.col1.y * a.x + mat.col2.y * a.y;
			return(result);
		}

		//
		// Vec3f functions (Do not depend on the operators)
		//
		inline bool IsEqual(const Vec3f &a, const Vec3f &b, const f32 tolerance) {
			bool result =
				IsEqual(a.x, b.x, tolerance) &&
				IsEqual(a.y, b.y, tolerance) &&
				IsEqual(a.z, b.z, tolerance);
			return(result);
		}

		inline f32 Dot(const Vec3f &a, const Vec3f &b) {
			f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
			return(result);
		}

		inline f32 LengthSquared(const Vec3f &vec) {
			f32 result = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
			return(result);
		}
		inline f32 Length(const Vec3f &vec) {
			f32 result = (f32)sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
			return(result);
		}

		inline f32 DistanceSquared(const Vec3f &a, const Vec3f &b) {
			f32 result = (a.x - b.x) * (a.y - b.y) * (a.z - b.z);
			return(result);
		}
		inline f32 Distance(const Vec3f &a, const Vec3f &b) {
			f32 result = (f32)sqrtf((a.x - b.x) * (a.y - b.y) * (a.z - b.z));
			return(result);
		}

		inline Vec3f Normalize(const Vec3f &vec) {
			f32 len = Length(vec);
			f32 invLen = len != 0.0f ? 1.0f / len : 1.0f;
			Vec3f result = Vec3f(vec.x * invLen, vec.y * invLen, vec.z * invLen);
			return(result);
		}

		inline Vec3f Minimum(const Vec3f &a, const Vec3f &b) {
			Vec3f result;
			result.x = Minimum(a.x, b.x);
			result.y = Minimum(a.y, b.y);
			result.z = Minimum(a.z, b.z);
			return(result);
		}

		inline Vec3f Maximum(const Vec3f &a, const Vec3f &b) {
			Vec3f result;
			result.x = Maximum(a.x, b.x);
			result.y = Maximum(a.y, b.y);
			result.z = Maximum(a.z, b.z);
			return(result);
		}

		inline Vec3f Lerp(const Vec3f &a, const f32 t, const Vec3f &b) {
			Vec3f result;
			result.x = Lerp(a.x, t, b.x);
			result.y = Lerp(a.y, t, b.y);
			result.z = Lerp(a.z, t, b.z);
			return(result);
		}

		inline Vec3f Clamp(const Vec3f &value, const Vec3f &min, const Vec3f &max) {
			Vec3f result;
			result.x = Clamp(value.x, min.x, max.x);
			result.y = Clamp(value.y, min.y, max.y);
			result.z = Clamp(value.z, min.z, max.z);
			return(result);
		}

		inline Vec3f Hadamard(const Vec3f &a, const Vec3f &b) {
			Vec3f result = Vec3f(a.x * b.x, a.y * b.y, a.z * b.z);
			return(result);
		}

		//
		// Vec3f operators (+, -, *, += etc.)
		//
		inline Vec3f operator + (const Vec3f &a, const Vec3f &b) {
			Vec3f result = Vec3f();
			result.x = a.x + b.x;
			result.y = a.y + b.y;
			result.y = a.z + b.z;
			return(result);
		}
		inline Vec3f &operator += (Vec3f &left, const Vec3f &right) {
			left.x += right.x;
			left.y += right.y;
			left.z += right.z;
			return(left);
		}
		inline Vec3f operator - (const Vec3f &a, const Vec3f &b) {
			Vec3f result = Vec3f();
			result.x = a.x - b.x;
			result.y = a.y - b.y;
			result.y = a.z - b.z;
			return(result);
		}
		inline Vec3f &operator -= (Vec3f &left, const Vec3f &right) {
			left.x -= right.x;
			left.y -= right.y;
			left.z -= right.z;
			return(left);
		}
		inline Vec3f operator * (const f32 scalar, const Vec3f &vec) {
			Vec3f result = Vec3f();
			result.x = vec.x * scalar;
			result.y = vec.y * scalar;
			result.z = vec.z * scalar;
			return(result);
		}
		inline Vec3f operator * (const Vec3f &vec, const f32 scalar) {
			Vec3f result = scalar * vec;
			return(result);
		}
		inline Vec3f &operator *= (Vec3f &left, const f32 right) {
			left.x *= right;
			left.y *= right;
			left.z *= right;
			return(left);
		}
		inline Vec3f operator - (const Vec3f &v) {
			Vec3f result = { -v.x, -v.y, -v.z };
			return(result);
		}

		//
		// Mat2f operators
		//
		inline Mat2f operator *(const Mat2f &a, const Mat2f &b) {
			Mat2f result;
			result.col1 = b.col1 * a;
			result.col2 = b.col2 * a;
			return(result);
		}

		//
		// Mat2f operations (Depends on operators)
		//
		inline Mat2f Transpose(const Mat2f &mat) {
			Mat2f result;
			result.m[0] = mat.col1.x;
			result.m[1] = mat.col2.x;
			result.m[2] = mat.col1.y;
			result.m[3] = mat.col2.y;
			return(result);
		}

		//
		// Mat4f operators
		//

		// @NOTE: Fastest SIMD mat4 mult: http://stackoverflow.com/questions/18499971/efficient-4x4-matrix-multiplication-c-vs-assembly
		inline Mat4f operator *(const Mat4f &a, const Mat4f &b) {
			Mat4f result = Mat4f(1.0f);
#if MATH_ENABLE_SIMD
			__m128 simd128x4[4];
			simd128x4[0] = _mm_load_ps(&a.m[0]);
			simd128x4[1] = _mm_load_ps(&a.m[4]);
			simd128x4[2] = _mm_load_ps(&a.m[8]);
			simd128x4[3] = _mm_load_ps(&a.m[12]);
			for (int i = 0; i < 4; i++) {
				__m128 brod1 = _mm_set1_ps(b.m[4 * i + 0]);
				__m128 brod2 = _mm_set1_ps(b.m[4 * i + 1]);
				__m128 brod3 = _mm_set1_ps(b.m[4 * i + 2]);
				__m128 brod4 = _mm_set1_ps(b.m[4 * i + 3]);
				__m128 row = _mm_add_ps(
					_mm_add_ps(
					_mm_mul_ps(brod1, simd128x4[0]),
					_mm_mul_ps(brod2, simd128x4[1])),
					_mm_add_ps(
					_mm_mul_ps(brod3, simd128x4[2]),
					_mm_mul_ps(brod4, simd128x4[3])));
				_mm_store_ps(&result.m[4 * i], row);
			}
#else
			for (u32 i = 0; i < 16; i += 4) {
				for (u32 j = 0; j < 4; ++j) {
					result.m[i + j] =
						(b.m[i + 0] * a.m[j + 0])
						+ (b.m[i + 1] * a.m[j + 4])
						+ (b.m[i + 2] * a.m[j + 8])
						+ (b.m[i + 3] * a.m[j + 12]);
				}
			}
#endif
			return(result);
		}

		//
		// Mat4f operations (Depends on operators)
		//
		inline Mat4f Transpose(const Mat4f &mat) {
			Mat4f result;

			result.m[0] = mat.col1.x;
			result.m[1] = mat.col2.x;
			result.m[2] = mat.col3.x;
			result.m[3] = mat.col4.x;

			result.m[4] = mat.col1.y;
			result.m[5] = mat.col2.y;
			result.m[6] = mat.col3.y;
			result.m[7] = mat.col4.y;

			result.m[8] = mat.col1.z;
			result.m[9] = mat.col2.z;
			result.m[10] = mat.col3.z;
			result.m[11] = mat.col4.z;

			result.m[12] = mat.col1.w;
			result.m[13] = mat.col2.w;
			result.m[14] = mat.col3.w;
			result.m[15] = mat.col4.w;

			return(result);
		}

		//
		// Color
		//

		constexpr f32 INV255 = 1.0f / 255.0f;

		inline Vec4f AlphaToLinear(u8 alpha) {
			f32 a = alpha * INV255;
			Vec4f result = Vec4f(1, 1, 1, a);
			return(result);
		}

		inline Vec4f RGBAToLinear(u32 rgba) {
			Pixel pixel = RGBAToPixel(rgba);
			Vec4f result = Vec4f(pixel.r * INV255, pixel.g * INV255, pixel.b * INV255, pixel.a * INV255);
			return(result);
		}

		inline u32 LinearToRGBA(const Vec4f &linear) {
			u8 r = (u8)((linear.x * 255.0f) + 0.5f);
			u8 g = (u8)((linear.y * 255.0f) + 0.5f);
			u8 b = (u8)((linear.z * 255.0f) + 0.5f);
			u8 a = (u8)((linear.w * 255.0f) + 0.5f);
			u32 result = RGBA(r, g, b, a);
			return(result);
		}
	};
};

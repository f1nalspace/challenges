#include "final_maths.h"

namespace finalspace {
	namespace maths {
		//
		// Vec2f
		//
		const Vec2f Vec2f::Up = Vec2f(0, 1);
		const Vec2f Vec2f::Down = Vec2f(0, -1);
		const Vec2f Vec2f::Left = Vec2f(-1, 0);
		const Vec2f Vec2f::Right = Vec2f(1, 0);

		//
		// Vec3f
		//
		const Vec3f Vec3f::Up = Vec3f(0, 1, 0);
		const Vec3f Vec3f::Down = Vec3f(0, -1, 0);
		const Vec3f Vec3f::Left = Vec3f(-1, 0, 0);
		const Vec3f Vec3f::Right = Vec3f(1, 0, 0);

		//
		// Mat2f
		//
		static const f32 MAT2_IDENTITY[4] = {
			1, 0,
			0, 1,
		};
		const Mat2f Mat2f::Identity = Mat2f(MAT2_IDENTITY);

		//
		// Mat4f
		//
		static const f32 MAT4_IDENTITY[16] = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
		const Mat4f Mat4f::Identity = Mat4f(MAT4_IDENTITY);
	}
}
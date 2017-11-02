#pragma once

#include "final_types.h"
#include "final_maths.h"
#include <vector>

using namespace fs::maths;

namespace fs {
	namespace collisions {
		struct Ray2D {
			Vec2f start;
			Vec2f end;
			f32 tMin;

			Ray2D(const Vec2f &start, const Vec2f &end, const f32 tMin) {
				this->start = start;
				this->end = end;
				this->tMin = tMin;
			}

			Ray2D(const Vec2f &start, const Vec2f &direction, const f32 length, const f32 tMin) {
				this->start = start;
				this->end = start + direction * length;
				this->tMin = tMin;
			}
		};

		struct Quad {
			Vec2f center;
			Vec2f ext;
		};

		struct LineCastResult {
			Vec2f surfaceNormal;
			b32 isHit;
			f32 tMin;
		};

		struct DeltaPlane2D {
			// @NOTE: Plane offset
			f32 plane;
			// @NOTE: Relative position in minkowski space
			f32 relX;
			f32 relY;
			// @NOTE: Delta movement
			f32 deltaX;
			f32 deltaY;
			// @NOTE: Min/Max corners
			f32 minY;
			f32 maxY;
			// @NOTE: Surface normal
			Vec2f normal;
		};

		struct IntersectionResult {
			Vec2f normal;
			f32 tMin;
			b32 wasHit;
		};

		extern IntersectionResult IntersectLines(const f32 tmin, const f32 epsilon, const u32 planeCount, const DeltaPlane2D *planes);

		extern void BresenhamLine(s32 x0, s32 y0, s32 x1, s32 y1, f32 width, std::vector<Vec2i> &out);

		extern LineCastResult LineCastQuad(const Ray2D &ray, const Quad &quad);
	};
};
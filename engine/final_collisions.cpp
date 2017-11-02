#include "final_collisions.h"

#include "final_mem.h"

namespace fs {
	namespace collisions {
		extern void BresenhamLine(s32 x0, s32 y0, s32 x1, s32 y1, f32 wd, std::vector<Vec2i> &out) {
			s32 dx = Absolute(x1 - x0), sx = x0 < x1 ? 1 : -1;
			s32 dy = Absolute(y1 - y0), sy = y0 < y1 ? 1 : -1;
			s32 err = dx - dy, e2, x2, y2;
			f32 ed = dx + dy == 0 ? 1 : SquareRoot((f32)dx*dx + (f32)dy*dy);
			for (wd = (wd + 1) / 2; ; ) {
				out.emplace_back(Vec2i(x0, y0));
				e2 = err; x2 = x0;
				// x step
				if (2 * e2 >= -dx) {
					for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
						out.emplace_back(Vec2i(x0, y2 += sy));
					if (x0 == x1) break;
					e2 = err; err -= dy; x0 += sx;
				}
				// y step
				if (2 * e2 <= dy) {
					for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
						out.emplace_back(Vec2i(x2 += sx, y0));
					if (y0 == y1) break;
					err += dx; y0 += sy;
				}
			}
		}

		extern IntersectionResult IntersectLines(const f32 tmin, const f32 epsilon, const u32 planeCount, const DeltaPlane2D *planes) {
			IntersectionResult result = {};
			result.tMin = tmin;
			for (u32 planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
				const DeltaPlane2D &plane = planes[planeIndex];
				if (plane.deltaX != 0.0f) {
					f32 f = (plane.plane - plane.relX) / plane.deltaX;
					f32 y = plane.relY + f*plane.deltaY;
					if ((f >= 0.0f) && (result.tMin > f)) {
						if ((y >= plane.minY) && (y <= plane.maxY)) {
							result.tMin = Maximum(0.0f, f - epsilon);
							result.normal = plane.normal;
							result.wasHit = true;
						}
					}
				}
			}

			return(result);
		}

		extern LineCastResult LineCastQuad(const Ray2D &ray, const Quad &quad) {
			LineCastResult result = {};
			result.tMin = ray.tMin;

			Vec2f delta = ray.end - ray.start;
			Vec2f rel = ray.start - quad.center;
			Vec2f minCorner = -quad.ext;
			Vec2f maxCorner = quad.ext;

			DeltaPlane2D testSides[4];
			u32 sideCount = 0;
			testSides[sideCount++] = { minCorner.x, rel.x, rel.y, delta.x, delta.y, minCorner.y, maxCorner.y,{ -1, 0 } };
			testSides[sideCount++] = { maxCorner.x, rel.x, rel.y, delta.x, delta.y, minCorner.y, maxCorner.y,{ 1, 0 } };
			testSides[sideCount++] = { minCorner.y, rel.y, rel.x, delta.y, delta.x, minCorner.x, maxCorner.x,{ 0, -1 } };
			testSides[sideCount++] = { maxCorner.y, rel.y, rel.x, delta.y, delta.x, minCorner.x, maxCorner.x,{ 0, 1 } };

			IntersectionResult intersectionResult = IntersectLines(ray.tMin, 0.0f, sideCount, testSides);
			if (intersectionResult.wasHit && intersectionResult.tMin < result.tMin) {
				result.isHit = true;
				result.tMin = intersectionResult.tMin;
				result.surfaceNormal = intersectionResult.normal;
			}

			return(result);
		}
	}
}
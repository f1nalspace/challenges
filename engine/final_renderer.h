#pragma once
#include "final_types.h"
#include "final_maths.h"

namespace finalspace {
	inline namespace renderer {

		struct Texture {
			void *handle;
			u32 width;
			u32 height;
		};

		struct Viewport {
			Vec2i offset;
			Vec2i size;
		};

		struct RenderArea {
			Vec2f sourceMin;
			Vec2f sourceMax;
			Vec2f targetMin;
			Vec2f targetMax;
			Vec2f sourceScale;
			Vec2f targetScale;

			inline Vec2f Project(const Vec2f &sourcePos) {
				Vec2f percentage = Vec2f((sourcePos.x - sourceMin.x) * sourceScale.x, (sourcePos.y - sourceMin.y) * sourceScale.y);
				f32 x = Lerp(targetMin.x, percentage.x, targetMax.x);
				f32 y = Lerp(targetMin.y, percentage.y, targetMax.y);
				Vec2f result = Vec2f(x, y);
				return(result);
			}

			inline Vec2f Unproject(const Vec2f &targetPos) {
				Vec2f percentage = Vec2f((targetPos.x - targetMin.x) * targetScale.x, (targetPos.y - targetMin.y) * targetScale.y);
				f32 x = Lerp(sourceMin.x, percentage.x, sourceMax.x);
				f32 y = Lerp(sourceMin.y, percentage.y, sourceMax.y);
				Vec2f result = Vec2f(x, y);
				return(result);
			}
		};

		inline RenderArea CalculateRenderArea(const Vec2f &sourceMin, const Vec2f &sourceMax, const Vec2f &targetMin, const Vec2f &targetMax, const bool letterBoxed) {
			// @TODO: Do we want to allow source min/max to be flipped as well
			assert(sourceMin.x < sourceMax.x);
			assert(sourceMin.y < sourceMax.y);

			RenderArea result = {};

			Vec2f sourceRange = Absolute(sourceMax - sourceMin);
			Vec2f targetRange = Absolute(targetMax - targetMin);

			result.sourceMin = sourceMin;
			result.sourceMax = sourceMax;


			if (letterBoxed) {
				// @NOTE: Letterbox calculation is done in absolute range always
				const f32 sourceAspect = sourceRange.w / sourceRange.h;
				Vec2f newTargetSize = Vec2f(targetRange.w, targetRange.w / sourceAspect);
				if (newTargetSize.h > targetRange.h) {
					newTargetSize.h = targetRange.h;
					newTargetSize.w = targetRange.h * sourceAspect;
				}
				Vec2f newTargetOffset = Vec2f((targetRange.w - newTargetSize.w) / 2.0f, (targetRange.h - newTargetSize.h) / 2.0f);
				targetRange = newTargetSize;

				// Nuisance, but this works
				if (targetMin.x < targetMax.x) {
					result.targetMin.x = targetMin.x + newTargetOffset.x;
					result.targetMax.x = targetMax.x - newTargetOffset.x;
					targetRange.x = newTargetSize.w;
				} else {
					result.targetMin.x = targetMin.x - newTargetOffset.x;
					result.targetMax.x = targetMax.x + newTargetOffset.x;
					targetRange.y = -newTargetSize.h;
				}

				if (targetMin.y < targetMax.y) {
					result.targetMin.y = targetMin.y + newTargetOffset.y;
					result.targetMax.y = targetMax.y - newTargetOffset.y;
					targetRange.y = newTargetSize.h;
				} else {
					result.targetMin.y = targetMin.y - newTargetOffset.y;
					result.targetMax.y = targetMax.y + newTargetOffset.y;
					targetRange.y = -newTargetSize.h;
				}
			} else {
				result.targetMin = targetMin;
				result.targetMax = targetMax;
			}

			result.sourceScale = Vec2f(1.0f / sourceRange.x, 1.0f / sourceRange.y);
			result.targetScale = Vec2f(1.0f / targetRange.x, 1.0f / targetRange.y);

			return(result);
		}

		class Renderer {
		public:
			Mat4f viewProjection;
			Viewport viewport;
			Vec2i windowSize;
			f32 viewScale;
			Vec2f viewSize;
			virtual void *AllocateTexture(const u32 width, const u32 height, void *data) = 0;
			virtual void BeginFrame() = 0;
			virtual void EndFrame() = 0;
			virtual void Update(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio) = 0;
			virtual Vec2f Unproject(const Vec2i &windowPos) = 0;
			virtual void DrawSprite(const Vec2f &pos, const Vec2f &ext, const Vec4f &color, const Texture &texture, const Vec2f &uvMin = Vec2f(0.0f, 0.0f), const Vec2f &uvMax = Vec2f(1.0f, 1.0f)) = 0;
			virtual void DrawRectangle(const Vec2f &pos, const Vec2f &ext, const Vec4f &color = Vec4f::White, const bool isFilled = true) = 0;
			Renderer() {}
			virtual ~Renderer() {}
		};

	};
};

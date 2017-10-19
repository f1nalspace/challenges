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

			inline Vec2f Project(const Vec2f &sourcePos) {
				Vec2f invScale = Vec2f(1.0f / sourceScale.x, 1.0f / sourceScale.y);
				Vec2f result = targetMin + Hadamard(sourcePos - sourceMin, invScale);
				return(result);
			}

			inline Vec2f Unproject(const Vec2f &targetPos) {
				Vec2f result = sourceMin + Vec2f((targetPos.x - targetMin.x) * sourceScale.x, (targetPos.y - targetMin.y) * sourceScale.y);
				return(result);
			}
		};

		inline RenderArea CalculateRenderArea(const Vec2f &sourceMin, const Vec2f &sourceMax, const Vec2f &targetMin, const Vec2f &targetMax, const bool letterBoxed) {
			RenderArea result = {};

			const Vec2f sourceRange = sourceMax - sourceMin;
			Vec2f targetRange = targetMax - targetMin;

			result.sourceMin = sourceMin;
			result.sourceMax = sourceMax;

			if (letterBoxed) {
				const f32 sourceAspect = sourceRange.w / sourceRange.h;
				Vec2f newTargetSize = Vec2f(targetRange.w, targetRange.w / sourceAspect);
				if (newTargetSize.h > targetRange.h) {
					newTargetSize.h = targetRange.h;
					newTargetSize.w = targetRange.h * sourceAspect;
				}
				Vec2f newTargetOffset = Vec2f((targetRange.w - newTargetSize.w) / 2.0f, (targetRange.h - newTargetSize.h) / 2.0f);
				targetRange = newTargetSize;

				result.targetMin = targetMin + newTargetOffset;
				result.targetMax = targetMin + newTargetOffset + newTargetSize;
			} else {
				result.targetMin = targetMin;
				result.targetMax = targetMax;
			}

			f32 scaleX = (sourceRange.x) / (targetRange.x);
			f32 scaleY = (sourceRange.y) / (targetRange.y);
			result.sourceScale = Vec2f(scaleX, scaleY);

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

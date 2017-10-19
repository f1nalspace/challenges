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

		inline RenderArea CreateRenderArea(const Vec2f &sourceMin, const Vec2f &sourceMax, const Vec2f &targetMin, const Vec2f &targetMax) {
			RenderArea result = {};

			Vec2f sourceSize = sourceMax - sourceMin;
			f32 sourceAspect = sourceSize.w / sourceSize.h;

			result.sourceMin = sourceMin;
			result.sourceMax = sourceMax;
			result.targetMin = targetMin;
			result.targetMax = targetMax;

			Vec2f sourceRange = sourceMax - sourceMin;
			Vec2f targetRange = targetMax - targetMin;

			f32 scaleX = (sourceRange.x) / (targetRange.x);
			f32 scaleY = (sourceRange.y) / (targetRange.y);

			result.sourceScale = Vec2f(scaleX, scaleY);

#if 0
			renderArea.screenSize = Vec2f(screenWidth, screenHeight);
			renderArea.halfAreaSize = Vec2f(halfGameWidth, halfGameHeight);
			renderArea.scale = (f32)screenWidth / (halfGameWidth * 2.0f);
			Vec2f viewportSize = Vec2f(screenWidth, screenWidth / aspectRatio);
			if (viewportSize.h > screenHeight) {
				viewportSize.h = screenHeight;
				viewportSize.w = viewportSize.h * aspectRatio;
				renderArea.scale = (f32)viewportSize.w / (halfGameWidth * 2.0f);
			}
			Vec2f viewportOffset = Vec2f((screenWidth - viewportSize.w) / 2.0f, (screenHeight - viewportSize.h) / 2.0f);
			renderArea.viewportSize = viewportSize;
			renderArea.viewportOffset = viewportOffset;
#endif
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
			virtual void DrawRectangle(const Vec2f &pos, const Vec2f &ext, const Vec4f &color = Vec4f::White, const bool isFilled = true) = 0;
			virtual void DrawSprite(const Vec2f &pos, const Vec2f &ext, const Texture &texture) = 0;
			Renderer() {}
			virtual ~Renderer() {}
		};

	};
};

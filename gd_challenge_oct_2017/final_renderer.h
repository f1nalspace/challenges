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
			Vec2f screenSize;
			Vec2f areaSize;
			Vec2f viewportOffset;
			Vec2f viewportSize;
			f32 scale;
		};

		inline void UpdateRenderArea(RenderArea &renderArea, const float screenWidth, const float screenHeight, const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio) {
			renderArea.screenSize = Vec2f(screenWidth, screenHeight);
			renderArea.areaSize = Vec2f(halfGameWidth, halfGameHeight) * 2.0f;
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
		}

		class Renderer {
		public:
			Mat4f viewProjection;
			Viewport viewport;
			Vec2i windowSize;
			f32 viewScale;
			Vec2f viewSize;
			void UpdateRenderState(RenderArea &area);
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

#pragma once

#include <final_platform_layer.hpp>
#include "final_types.h"
#include "final_renderer.h"

namespace fs {
	namespace renderer {
		class OpenGLRenderer : public Renderer {
		public:
			void *AllocateTexture(const u32 width, const u32 height, void *data) override;
			void BeginFrame() override;
			void EndFrame() override;
			void Update(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio) override;
			Vec2f Unproject(const Vec2i &windowPos) override;
			void DrawSprite(const Vec2f &pos, const Vec2f &ext, const Vec4f &color, const Texture &texture, const Vec2f &uvMin, const Vec2f &uvMax) override;
			void DrawRectangle(const Vec2f &pos, const Vec2f &ext, const Vec4f &color, const bool isFilled, const f32 lineWidth) override;
			void DrawLine(const Vec2f &a, const Vec2f &b, const Vec4f &color, const f32 lineWidth) override;
			void DrawCircle(const Vec2f &center, const f32 radius, const Vec4f &color, const bool isFilled, const u32 segmentCount, const f32 lineWidth) override;
			OpenGLRenderer() : Renderer() {
			}
			~OpenGLRenderer() {
			};
		};
	};
};
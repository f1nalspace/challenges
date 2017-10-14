#pragma once

#include <final_platform_layer.hpp>
#include "final_types.h"
#include "final_render.h"

namespace finalspace {
	namespace renderer {
		class OpenGLRenderer : Renderer {
		public:
			void *AllocateTexture(const u32 width, const u32 height, void *data) override;
			void BeginFrame() override;
			void EndFrame() override;
			void Update(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio) override;
			Vec2f Unproject(const Vec2i &windowPos) override;
			void DrawRectangle(const Vec2f &pos, const Vec2f &ext, const Vec4f &color, const bool isFilled) override;
			void DrawSprite(const Vec2f &pos, const Vec2f &ext, const Texture &texture) override;
			OpenGLRenderer() : Renderer() {}
			~OpenGLRenderer() {};
		};
	};
};

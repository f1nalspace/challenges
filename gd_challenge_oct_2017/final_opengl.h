#pragma once

#include <final_platform_layer.hpp>
#include "final_types.h"
#include "final_render.h"

namespace finalspace {
	namespace renderer {
		class OpenGLRenderer : Renderer {
		public:
			void *AllocateTexture(const u32 width, const u32 height, void *data) override;
			void BeginFrame(const f32 halfGameWidth, const f32 halfGameHeight, const f32 aspectRatio) override;
			void EndFrame() override;
			OpenGLRenderer() : Renderer() {}
			~OpenGLRenderer() {};
		};
	};
};

#pragma once

#include "final_types.h"
#include "final_maths.h"

using namespace finalspace::maths;

namespace finalspace {
	namespace renderer {

		struct Texture {
			void *handle;
			u32 width;
			u32 height;
		};

		class Renderer {
		public:
			Mat4f viewProjection;
			Vec2i windowSize;
			virtual void *AllocateTexture(const u32 width, const u32 height, void *data) = 0;
			virtual void BeginFrame(const f32 halfGameWidth, const f32 halfGameHeight) = 0;
			virtual void EndFrame() = 0;
			Renderer() {}
			virtual ~Renderer() {}
		};

	};
};

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

		struct Viewport {
			Vec2i offset;
			Vec2i size;
		};

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
			Renderer() {}
			virtual ~Renderer() {}
		};

	};
};

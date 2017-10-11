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

		struct RenderState {
			Vec2i windowSize;
		};

	};
};

#pragma once

#include <final_platform_layer.hpp>
#include "final_types.h"

namespace finalspace {
	class OpenGLRenderer {
	public:
		static void *AllocateTexture(const u32 width, const u32 height, void *data);
	};
};

#pragma once

#include "final_types.h"
#include "final_maths.h"
#include "final_input.h"
#include "final_render.h"

using namespace finalspace::inputs;
using namespace finalspace::maths;
using namespace finalspace::renderer;

namespace finalspace {

	namespace games {

		struct Game {
			Vec2f position;

			void Init();
			void Update(const Input &input);
			void Render(RenderState &render);
		};

	};

};
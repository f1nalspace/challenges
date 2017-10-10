#pragma once

#include <vector>

#include "final_types.h"
#include "final_maths.h"
#include "final_input.h"
#include "final_render.h"

using namespace finalspace::inputs;
using namespace finalspace::maths;
using namespace finalspace::renderer;

namespace finalspace {

	namespace games {

		struct Player {
			Vec2f position;
			Vec2f velocity;
			Vec2f ext;
		};

		struct Wall {
			Vec2f position;
			Vec2f ext;
		};

		struct Game {
			Vec2f gravity;
			Player player;
			std::vector<Wall> walls;

			void Init();
			void Update(const Input &input);
			void Render(RenderState &render);
		};

	};

};
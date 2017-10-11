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

		struct Entity {
			Vec2f position;
			Vec2f velocity;
			Vec2f ext;
			b32 isGrounded;
			f32 horizontalSpeed;
			f32 horizontalDrag;
			b32 canJump;
			f32 jumpPower;
		};

		struct Wall {
			Vec2f position;
			Vec2f ext;
			b32 isPlatform;
		};

		struct Game {
			Vec2f gravity;
			std::vector<Entity> players;
			std::vector<Wall> walls;

			void Init();
			void Update(const Input &input);
			void Render(RenderState &render);
		};

	};

};
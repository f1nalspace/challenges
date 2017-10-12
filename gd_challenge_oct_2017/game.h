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

		enum class EntityType {
			Player,
		};

		struct Entity {
			Vec2f position;
			Vec2f velocity;
			Vec2f ext;
			EntityType type;
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
			const f32 GameAspect = 16.0f / 9.0f;
			const f32 GameWidth = 20.0f;
			const f32 GameHeight = GameWidth / GameAspect;
			const f32 HalfGameWidth = GameWidth * 0.5f;
			const f32 HalfGameHeight = GameHeight * 0.5f;

			Vec2f gravity;
			std::vector<Entity> players;
			std::vector<Wall> walls;

			Texture texture;

			void Init();
			void Release();
			void Update(const Input &input);
			void Render(RenderState &render);
		};

	};

};
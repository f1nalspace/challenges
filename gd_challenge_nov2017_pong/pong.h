#pragma once

#include <vector>

#include "final_game.h"
#include "final_maths.h"

using namespace fs::maths;

namespace fs {
	namespace games {
		struct Entity {
			Vec2f position = Vec2f();
			Vec2f velocity = Vec2f();
			Vec2f acceleration = Vec2f();
			Vec2f delta = Vec2f();
		};

		struct Ball : Entity {
			f32 radius = 0.5f;
		};

		struct Paddle : Entity {
			Vec2f ext = Vec2f(0.25f, 0.9f);
		};

		struct Plane {
			Vec2f normal = Vec2f();
			f32 distance = 0.0f;
			f32 visualLength = 0.0f;
			Plane() = default;
			Plane(const Plane &other) {
				this->normal = other.normal;
				this->distance = other.distance;
				this->visualLength = other.visualLength;
			}
			Plane(const Vec2f &normal, const f32 distance, const f32 visualLength) {
				this->normal = normal;
				this->distance = distance;
				this->visualLength = visualLength; 
			}
		};

		struct Pong : BaseGame {
		public:
			static constexpr f32 GAME_ASPECT = 16.0f / 9.0f;
			static constexpr f32 GAME_WIDTH = 20.0f;
			static constexpr f32 GAME_HEIGHT = GAME_WIDTH / GAME_ASPECT;
			static constexpr f32 GAME_HALF_WIDTH = GAME_WIDTH * 0.5f;
			static constexpr f32 GAME_HALF_HEIGHT = GAME_HEIGHT * 0.5f;

			Ball ball;
			Paddle paddle;
			std::vector<Plane> planes;

			Pong();
			~Pong() override;
			void Init() override;
			void Release() override;
			void HandleInput(const Input &input) override;
			void Update(const Input &input) override;
			void Render(const Input &input) override;
		};
	};
};
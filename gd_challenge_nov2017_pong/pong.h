#pragma once

#include <final_game.h>

namespace fs {
	namespace games {
		struct Pong : BaseGame {
		public:
			static constexpr f32 GAME_ASPECT = 16.0f / 9.0f;
			static constexpr f32 GAME_WIDTH = 20.0f;
			static constexpr f32 GAME_HEIGHT = GAME_WIDTH / GAME_ASPECT;

			Pong();
			~Pong() override;
			void Init() override;
			void Release() override;
			void HandleInput(const Input &input) override;
			void Update(const Input &input) override;
			void Render() override;
		};
	};
};
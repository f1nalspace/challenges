#pragma once

#include <final_game.h>

namespace fs {
	namespace games {
		struct Pong : BaseGame {
		public:
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
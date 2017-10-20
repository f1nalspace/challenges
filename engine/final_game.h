#pragma once

#include "final_renderer.h"
#include "final_input.h"

namespace finalspace {

	inline namespace games {

		class BaseGame {
		protected:
			Renderer &renderer;
			bool exitRequested;
		public:
			BaseGame(Renderer &renderer) :
				renderer(renderer),
				exitRequested(false) {
			}
			virtual void Init() = 0;
			virtual void Release() = 0;
			virtual void HandleInput(const Input &input) = 0;
			virtual void Update(const Input &input) = 0;
			virtual void Render() = 0;
			virtual ~BaseGame() {
			}
			inline bool IsExitRequested() const {
				return exitRequested;
			}
		};
	}
}
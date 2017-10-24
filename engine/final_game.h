#pragma once

#include "final_renderer.h"
#include "final_input.h"

using namespace fs::renderer;
using namespace fs::inputs;

namespace fs {
	namespace games {
		class BaseGame {
		private:
			u32 initialWidth;
			u32 initialHeight;
		protected:
			Renderer *renderer;
			bool exitRequested;
		public:
			BaseGame() :
				renderer(nullptr),
				exitRequested(false),
				initialWidth(640),
				initialHeight(360) {
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
			inline u32 GetInitialWidth() const {
				return initialWidth;
			}
			inline u32 GetInitialHeight() const {
				return initialHeight;
			}
			inline void SetRenderer(Renderer *renderer) {
				this->renderer = renderer;
			}
		};
	};
};
#pragma once

#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"

using namespace finalspace::maths;
using namespace finalspace::utils;

namespace finalspace {

	namespace inputs {

		struct ButtonState {
			b32 isDown = false;
			s32 halfTransitionCount = 0;

			inline bool WasPressed() const {
				bool result = ((halfTransitionCount > 1) || ((halfTransitionCount == 1) && (isDown)));
				return(result);
			}

			ButtonState() {
			}
		};

		struct Controller {
			u32 playerIndex = 0;
			Vec2f movement = Vec2f();
			union {
				struct {
					ButtonState actionUp;
					ButtonState actionDown;
					ButtonState actionLeft;
					ButtonState actionRight;
				};
				ButtonState buttons[4] = {};
			};

			Controller() {
			}
		};

		struct Input {
			f32 deltaTime = 0.0f;
			u32 playerOneControllerIndex = 0;
			union {
				struct {
					Controller keyboard;
				};
				Controller controllers[1] = {};
			};

			Input() {
			}
		};

	};

};
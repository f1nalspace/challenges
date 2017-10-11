#pragma once

#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"

using namespace finalspace::maths;
using namespace finalspace::utils;

namespace finalspace {

	namespace inputs {

		struct ButtonState {
			b32 isDown;
			s32 halfTransitionCount;

			inline bool WasPressed() const {
				bool result = ((halfTransitionCount > 1) || ((halfTransitionCount == 1) && (isDown)));
				return(result);
			}
		};

		struct Controller {
			u32 playerIndex;
			Vec2f movement;
			union {
				struct {
					ButtonState actionUp;
					ButtonState actionDown;
					ButtonState actionLeft;
					ButtonState actionRight;
				};
				ButtonState buttons[4];
			};
		};

		struct Input {
			f32 deltaTime;
			u32 playerOneControllerIndex;
			union {
				struct {
					Controller keyboard;
				};
				Controller controllers[1];
			};
		};

	};

};
#pragma once

#include "final_types.h"

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
			union {
				struct {
					ButtonState moveUp;
					ButtonState moveDown;
					ButtonState moveLeft;
					ButtonState moveRight;
				};
				ButtonState buttons[4];
			};
		};

		struct Input {
			f32 deltaTime;
			union {
				struct {
					Controller keyboard;
				};
				Controller controller[1];
			};
			u32 playerOneControllerIndex;
		};

	};

};
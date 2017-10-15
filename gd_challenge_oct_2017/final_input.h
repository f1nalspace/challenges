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
				bool result =
					((halfTransitionCount > 1) ||
					((halfTransitionCount == 1) && (isDown)));
				return(result);
			}

			ButtonState() {
			}
		};

		struct Controller {
			b32 isConnected = false;
			b32 isAnalog = false;
			Vec2f analogMovement = Vec2f();
			union {
				struct {
					ButtonState moveUp;
					ButtonState moveDown;
					ButtonState moveLeft;
					ButtonState moveRight;
					ButtonState actionUp;
					ButtonState actionDown;
					ButtonState actionLeft;
					ButtonState actionRight;
					ButtonState editorToggle;
				};
				ButtonState buttons[9] = {};
			};

			Controller() {
			}
		};

		struct Mouse {
			Vec2i pos = Vec2i();
			f32 wheelDelta = 0.0f;
			union {
				struct {
					ButtonState left;
					ButtonState middle;
					ButtonState right;
				};
				ButtonState buttons[3] = {};
			};

			Mouse() {}
		};

		struct Input {
			f32 deltaTime = 0.0f;
			union {
				struct {
					Controller keyboard;
					Controller gamepad[4];
				};
				Controller controllers[5] = {};
			};
			Mouse mouse = {};

			Input() {
			}
		};

	};

};
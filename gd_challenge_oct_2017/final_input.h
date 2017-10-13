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

		enum class ControllerState {
			None,
			Connected,
			Disconnected,
		};

		struct Controller {
			ControllerState state = ControllerState::None;
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
				};
				ButtonState buttons[8] = {};
			};

			Controller() {
			}
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

			Input() {
			}
		};

	};

};
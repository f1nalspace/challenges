#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

using namespace fpl;
using namespace fpl::window;
using namespace fpl::timings;
using namespace fpl::console;

#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"
#include "final_render.h"
#include "final_opengl.h"

// @TODO: Make this a full game independent platform layer
#include "game.h"

using namespace finalspace;
using namespace finalspace::games;
using namespace finalspace::maths;
using namespace finalspace::utils;
using namespace finalspace::renderer;

static void UpdateButtonState(const b32 isDown, ButtonState &targetButton) {
	if (isDown != targetButton.isDown) {
		targetButton.isDown = isDown;
		++targetButton.halfTransitionCount;
	}
}

int main(int argc, char **args) {
	if (InitPlatform(InitFlags::VideoOpenGL)) {
		SetWindowResizeable(true);
		SetWindowArea(1280, 720);

		// Init
		Renderer *renderer = (Renderer *)new OpenGLRenderer();
		Game *game = new Game();
		game->Init(*renderer);

		constexpr f32 targetDeltaTime = 1.0f / 60.0f;

		f64 lastTime = GetHighResolutionTimeInSeconds();
		f64 fpsTimerInSecs = GetHighResolutionTimeInSeconds();
		f64 frameAccumulator = targetDeltaTime;
		uint32_t frameCount = 0;
		uint32_t updateCount = 0;

		Input inputs[2] = {};
		Input *currentInput = &inputs[0];
		Input *prevInput = &inputs[1];

		// Loop
		bool isWindowActive = true;
		while (WindowUpdate()) {
			//
			// Window size
			//
			WindowSize windowArea = GetWindowArea();
			renderer->windowSize = Vec2i(windowArea.width, windowArea.height);

			//
			// Input
			//
			{
				// Remember previous keyboard state
				Controller *currentKeyboardController = &currentInput->keyboard;
				Controller *prevKeyboardController = &prevInput->keyboard;
				*currentKeyboardController = {};
				if (isWindowActive) {
					currentKeyboardController->isConnected = true;
					for (u32 buttonIndex = 0; buttonIndex < ArrayCount(currentKeyboardController->buttons); ++buttonIndex) {
						currentKeyboardController->buttons[buttonIndex].isDown = prevKeyboardController->buttons[buttonIndex].isDown;
					}
				}

				// Remember previous gamepad connected states
				for (u32 controllerIndex = 1; controllerIndex < ArrayCount(currentInput->controllers); ++controllerIndex) {
					Controller *currentGamepadController = &currentInput->controllers[controllerIndex];
					Controller *prevGamepadController = &prevInput->controllers[controllerIndex];
					currentGamepadController->isConnected = prevGamepadController->isConnected;
					currentGamepadController->isAnalog = prevGamepadController->isAnalog;
				}

				// Process events
				Event event;
				while (PollWindowEvent(event)) {
					switch (event.type) {
						case EventType::Window:
						{
							switch (event.window.type) {
								case WindowEventType::GotFocus:
									isWindowActive = true;
									break;
								case WindowEventType::LostFocus:
									isWindowActive = false;
									break;
							}
						} break;
						case EventType::Gamepad:
						{
							// @CLEANUP: For now we just use the device index, but later it should be "added" to the controllers array and remembered somehow
							u32 controllerIndex = 1 + event.gamepad.deviceIndex;
							assert(controllerIndex < ArrayCount(currentInput->controllers));

							Controller *gamepadController = &currentInput->controllers[controllerIndex];
							switch (event.gamepad.type) {
								case GamepadEventType::Connected:
								{
									gamepadController->isConnected = true;
								} break;
								case GamepadEventType::Disconnected:
								{
									gamepadController->isConnected = false;
								} break;
								case GamepadEventType::StateChanged:
								{
									GamepadState &padstate = event.gamepad.state;
									assert(gamepadController->isConnected);
									if (abs(padstate.leftStickX) > 0.0f || abs(padstate.leftStickY) > 0.0f) {
										gamepadController->isAnalog = true;
										gamepadController->analogMovement.x = padstate.leftStickX;
										gamepadController->analogMovement.y = padstate.leftStickY;
									} else {
										gamepadController->isAnalog = false;
										UpdateButtonState(padstate.dpadDown.isDown, gamepadController->moveDown);
										UpdateButtonState(padstate.dpadUp.isDown, gamepadController->moveUp);
										UpdateButtonState(padstate.dpadLeft.isDown, gamepadController->moveLeft);
										UpdateButtonState(padstate.dpadRight.isDown, gamepadController->moveRight);
									}

									UpdateButtonState(padstate.actionA.isDown, gamepadController->actionDown);
									UpdateButtonState(padstate.actionB.isDown, gamepadController->actionRight);
									UpdateButtonState(padstate.actionX.isDown, gamepadController->actionLeft);
									UpdateButtonState(padstate.actionY.isDown, gamepadController->actionUp);
								} break;
							}
						} break;
						case EventType::Keyboard:
						{
							switch (event.keyboard.type) {
								case KeyboardEventType::KeyDown:
								case KeyboardEventType::KeyUp:
								{
									b32 isDown = (event.keyboard.type == KeyboardEventType::KeyDown) ? 1 : 0;
									switch (event.keyboard.mappedKey) {
										case Key::Key_A:
										case Key::Key_Left:
											UpdateButtonState(isDown, currentKeyboardController->moveLeft);
											break;
										case Key::Key_D:
										case Key::Key_Right:
											UpdateButtonState(isDown, currentKeyboardController->moveRight);
											break;
										case Key::Key_W:
										case Key::Key_Up:
											UpdateButtonState(isDown, currentKeyboardController->moveUp);
											break;
										case Key::Key_S:
										case Key::Key_Down:
											UpdateButtonState(isDown, currentKeyboardController->moveDown);
											break;
										case Key::Key_Space:
											UpdateButtonState(isDown, currentKeyboardController->actionDown);
											break;
									}
								} break;
							}
						} break;
					}
				}
				currentInput->deltaTime = targetDeltaTime;
			}

			//
			// Update
			//
			{
				frameAccumulator = Clamp(frameAccumulator, 0.0, 0.5);
				while (frameAccumulator >= targetDeltaTime) {
					game->Update(*currentInput);
					++updateCount;
					frameAccumulator -= targetDeltaTime;
				}
			}

			//
			// Render
			//
			{
				game->Render(*renderer);
				WindowFlip();
				++frameCount;
			}

			//
			// Timings
			//
			{
				f64 frameEndTime = GetHighResolutionTimeInSeconds();
				f64 frameDuration = frameEndTime - lastTime;
				frameAccumulator += frameDuration;
				lastTime = frameEndTime;
				if (frameEndTime >= (fpsTimerInSecs + 1.0)) {
					fpsTimerInSecs = frameEndTime;
					ConsoleFormatOut("Fps: %d, Ups: %d\n", frameCount, updateCount);
					frameCount = 0;
					updateCount = 0;
				}
			}

			// Swap current and previous input
			Swap(currentInput, prevInput);
		}

		game->Release(*renderer);
		delete game;
		delete renderer;

		ReleasePlatform();
	}
}
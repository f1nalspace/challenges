#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

using namespace fpl;
using namespace fpl::window;
using namespace fpl::console;

#include "final_game.h"
#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"
#include "final_renderer.h"
#include "final_openglrenderer.h"

using namespace fs;
using namespace fs::maths;
using namespace fs::games;

#include "pong.h"

static void UpdateKeyboardButtonState(const b32 isDown, ButtonState &targetButton) {
	if (isDown != targetButton.isDown) {
		targetButton.isDown = isDown;
		++targetButton.halfTransitionCount;
	}
}

static void UpdateDigitalButtonState(const b32 isDown, const ButtonState &oldState, ButtonState &newState) {
	newState.isDown = isDown;
	newState.halfTransitionCount = (oldState.isDown != newState.isDown) ? 1 : 0;
}

static void ProcessEvents(Input *currentInput, Input *prevInput, bool &isWindowActive, Vec2i &lastMousePos) {
	Controller *currentKeyboardController = &currentInput->keyboard;
	Controller *prevKeyboardController = &prevInput->keyboard;

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
				assert(controllerIndex < utils::ArrayCount(currentInput->controllers));

				Controller *currentController = &currentInput->controllers[controllerIndex];
				Controller *prevController = &prevInput->controllers[controllerIndex];
				switch (event.gamepad.type) {
					case GamepadEventType::Connected:
					{
						currentController->isConnected = true;
					} break;
					case GamepadEventType::Disconnected:
					{
						currentController->isConnected = false;
					} break;
					case GamepadEventType::StateChanged:
					{
						GamepadState &padstate = event.gamepad.state;
						assert(currentController->isConnected);
						if (Absolute(padstate.leftStickX) > 0.0f || Absolute(padstate.leftStickY) > 0.0f) {
							currentController->isAnalog = true;
							currentController->analogMovement.x = padstate.leftStickX;
							currentController->analogMovement.y = padstate.leftStickY;
						} else {
							currentController->isAnalog = false;
							UpdateDigitalButtonState(padstate.dpadDown.isDown, prevController->moveDown, currentController->moveDown);
							UpdateDigitalButtonState(padstate.dpadUp.isDown, prevController->moveUp, currentController->moveUp);
							UpdateDigitalButtonState(padstate.dpadLeft.isDown, prevController->moveLeft, currentController->moveLeft);
							UpdateDigitalButtonState(padstate.dpadRight.isDown, prevController->moveRight, currentController->moveRight);
						}

						UpdateDigitalButtonState(padstate.actionA.isDown, prevController->actionDown, currentController->actionDown);
						UpdateDigitalButtonState(padstate.actionB.isDown, prevController->actionRight, currentController->actionRight);
						UpdateDigitalButtonState(padstate.actionX.isDown, prevController->actionLeft, currentController->actionLeft);
						UpdateDigitalButtonState(padstate.actionY.isDown, prevController->actionUp, currentController->actionUp);
					} break;
				}
			} break;

			case EventType::Mouse:
			{
				switch (event.mouse.type) {
					case MouseEventType::Move:
					{
						lastMousePos = currentInput->mouse.pos = Vec2i(event.mouse.mouseX, event.mouse.mouseY);
					} break;

					case MouseEventType::ButtonDown:
					case MouseEventType::ButtonUp:
					{
						bool isDown = event.mouse.type == MouseEventType::ButtonDown;
						if (event.mouse.mouseButton == MouseButtonType::Left) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.left);
						} else if (event.mouse.mouseButton == MouseButtonType::Right) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.right);
						} else if (event.mouse.mouseButton == MouseButtonType::Middle) {
							UpdateKeyboardButtonState(isDown, currentInput->mouse.middle);
						}
					} break;

					case MouseEventType::Wheel:
					{
						currentInput->mouse.wheelDelta = event.mouse.wheelDelta;
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
							case Key::Key_F1:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->editorToggle);
								break;
							case Key::Key_A:
							case Key::Key_Left:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveLeft);
								break;
							case Key::Key_D:
							case Key::Key_Right:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveRight);
								break;
							case Key::Key_W:
							case Key::Key_Up:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveUp);
								break;
							case Key::Key_S:
							case Key::Key_Down:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->moveDown);
								break;
							case Key::Key_Space:
								UpdateKeyboardButtonState(isDown, currentKeyboardController->actionDown);
								break;
						}
					} break;
				}
			} break;
		}
	}
}

int main(int argc, char **args) {
	BaseGame *game = new Pong();
	InitSettings platformSettings = InitSettings();
	platformSettings.window.windowWidth = game->GetInitialWidth();
	platformSettings.window.windowHeight = game->GetInitialHeight();
	platformSettings.video.isVSync = false;
	if (InitPlatform(InitFlags::VideoOpenGL, platformSettings)) {
		Renderer *renderer = (Renderer *)new OpenGLRenderer();

		game->SetRenderer(renderer);
		game->Init();

		constexpr f32 TargetDeltaTime = 1.0f / 60.0f;

		f64 lastTime = timings::GetHighResolutionTimeInSeconds();
		f64 fpsTimerInSecs = timings::GetHighResolutionTimeInSeconds();
		f64 frameAccumulator = TargetDeltaTime;
		uint32_t frameCount = 0;
		uint32_t updateCount = 0;

		Input inputs[2] = {};
		Input *currentInput = &inputs[0];
		Input *prevInput = &inputs[1];
		Vec2i lastMousePos = Vec2i(-1, -1);

		// Loop
		bool isWindowActive = true;
		while (!game->IsExitRequested() && WindowUpdate()) {
			//
			// Window size
			//
			WindowSize windowArea = GetWindowArea();
			renderer->windowSize = Vec2i(windowArea.width, windowArea.height);

			//
			// Input
			//
			{
				// Remember previous keyboard and mouse state
				Controller *currentKeyboardController = &currentInput->keyboard;
				Controller *prevKeyboardController = &prevInput->keyboard;
				Mouse *currentMouse = &currentInput->mouse;
				Mouse *prevMouse = &prevInput->mouse;
				*currentKeyboardController = {};
				*currentMouse = {};
				currentKeyboardController->isConnected = true;
				if (isWindowActive) {
					for (u32 buttonIndex = 0; buttonIndex < utils::ArrayCount(currentKeyboardController->buttons); ++buttonIndex) {
						currentKeyboardController->buttons[buttonIndex].isDown = prevKeyboardController->buttons[buttonIndex].isDown;
					}
					for (u32 buttonIndex = 0; buttonIndex < utils::ArrayCount(currentMouse->buttons); ++buttonIndex) {
						currentMouse->buttons[buttonIndex] = prevMouse->buttons[buttonIndex];
						currentMouse->buttons[buttonIndex].halfTransitionCount = 0;
					}
				}

				// Remember previous gamepad connected states
				for (u32 controllerIndex = 1; controllerIndex < utils::ArrayCount(currentInput->controllers); ++controllerIndex) {
					Controller *currentGamepadController = &currentInput->controllers[controllerIndex];
					Controller *prevGamepadController = &prevInput->controllers[controllerIndex];
					currentGamepadController->isConnected = prevGamepadController->isConnected;
					currentGamepadController->isAnalog = prevGamepadController->isAnalog;
				}

				// Remember previous mouse states
				currentInput->mouse.pos = lastMousePos;

				// Set time states
				currentInput->deltaTime = TargetDeltaTime;

				// Process events
				ProcessEvents(currentInput, prevInput, isWindowActive, lastMousePos);
			}

			//
			// Tick & Update
			//
			{
				game->HandleInput(*currentInput);
				frameAccumulator = Clamp(frameAccumulator, 0.0, 0.5);
				while (frameAccumulator >= TargetDeltaTime) {
					game->Update(*currentInput);
					++updateCount;
					frameAccumulator -= TargetDeltaTime;
				}
			}

			//
			// Render
			//
			{
				game->Render();
				WindowFlip();
				++frameCount;
			}

			//
			// Timings
			//
			{
				f64 frameEndTime = timings::GetHighResolutionTimeInSeconds();
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
			utils::Swap(currentInput, prevInput);
		}

		// Release resources
		game->Release();
		delete renderer;
		ReleasePlatform();
		delete game;
	}
}
#define GLEW_STATIC
#include <GL\glew.h>

#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

using namespace fpl;
using namespace fpl::window;
using namespace fpl::timings;
using namespace fpl::console;

#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"
#include "final_render.h"

// @TODO: Make this a full game independent platform layer
#include "game.h"

using namespace finalspace;
using namespace finalspace::games;
using namespace finalspace::maths;
using namespace finalspace::utils;
using namespace finalspace::renderer;

static void ProcessKeyboardButton(const b32 isDown, ButtonState &targetButton) {
	if (isDown != targetButton.isDown) {
		targetButton.isDown = isDown;
		++targetButton.halfTransitionCount;
	}
}

int main(int argc, char **args) {
	if (InitPlatform(InitFlags::VideoOpenGL)) {
		GLenum glewInitResult = glewInit();
		if (glewInitResult != GLEW_NO_ERROR) {
			// @TODO: Log error
		}

		SetWindowResizeable(true);
		SetWindowArea(1280, 720);

		// Init
		Game *game = new Game();
		game->Init();

		constexpr f32 targetDeltaTime = 1.0f / 60.0f;

		f64 lastTime = GetHighResolutionTimeInSeconds();
		f64 fpsTimerInSecs = GetHighResolutionTimeInSeconds();
		f64 frameAccumulator = targetDeltaTime;
		uint32_t frameCount = 0;
		uint32_t updateCount = 0;

		Input inputs[2] = {};
		Input *currentInput = &inputs[0];
		Input *prevInput = &inputs[1];

		RenderState renderState = {};

		// Loop
		while (WindowUpdate()) {
			//
			// Window size
			//
			WindowSize windowArea = GetWindowArea();
			renderState.windowSize = Vec2i(windowArea.width, windowArea.height);

			//
			// Input
			//
			{
				// Remember previous keyboard input
				Controller *currentKeyboardController = &currentInput->keyboard;
				Controller *prevKeyboardController = &prevInput->keyboard;
				*currentKeyboardController = {};
				for (u32 buttonIndex = 0; buttonIndex < ArrayCount(currentKeyboardController->buttons); ++buttonIndex) {
					currentKeyboardController->buttons[buttonIndex].isDown = prevKeyboardController->buttons[buttonIndex].isDown;
				}

				// Process events
				Event event;
				while (PollWindowEvent(&event)) {
					switch (event.type) {
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
											ProcessKeyboardButton(isDown, currentKeyboardController->moveLeft);
											break;
										case Key::Key_D:
										case Key::Key_Right:
											ProcessKeyboardButton(isDown, currentKeyboardController->moveRight);
											break;
										case Key::Key_W:
										case Key::Key_Up:
											ProcessKeyboardButton(isDown, currentKeyboardController->moveUp);
											break;
										case Key::Key_S:
										case Key::Key_Down:
											ProcessKeyboardButton(isDown, currentKeyboardController->moveDown);
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
				game->Render(renderState);
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

		delete game;

		ReleasePlatform();
	}
}
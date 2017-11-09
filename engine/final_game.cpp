#include "final_game.h"

#define FPL_AUTO_NAMESPACE 1
#include <final_platform_layer.hpp>

#include "final_types.h"
#include "final_maths.h"
#include "final_utils.h"
#include "final_renderer.h"
#include "final_openglrenderer.h"

namespace fs {
	namespace games {
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

	#if FS_ENABLE_IMGUI
		struct ImGUIState {
			int currentMousePosition[2] = { -1, -1 };
			bool currentMouseStates[3] = { 0 };
			float currentMouseWheelDelta = 0.0f;
			GLuint fontTextureId = 0;
		};

		static ImGUIState imGuiState = {};

		// @CLEANUP: Use the renderer instead - i dont want to call any opengl code here!
		static void ImGUIRenderDrawLists(ImDrawData* draw_data) {
			// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
			ImGuiIO& io = ImGui::GetIO();
			int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
			int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
			if (fb_width == 0 || fb_height == 0)
				return;
			draw_data->ScaleClipRects(io.DisplayFramebufferScale);

			// We are using the OpenGL fixed pipeline to make the example code simpler to read!
			// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
			GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
			GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
			glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnable(GL_TEXTURE_2D);
			//glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

			// Setup viewport, orthographic projection matrix
			glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			// Render command lists
		#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
			for (int n = 0; n < draw_data->CmdListsCount; n++) {
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
				const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
				glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
				glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
				glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, col)));

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback) {
						pcmd->UserCallback(cmd_list, pcmd);
					} else {
						glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
						glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
					}
					idx_buffer += pcmd->ElemCount;
				}
			}
		#undef OFFSETOF

			// Restore modified state
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glPopAttrib();
			glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
			glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
		}

		static void InitImGUI() {
			ImGuiIO& io = ImGui::GetIO();

			io.RenderDrawListsFn = ImGUIRenderDrawLists;
			io.IniFilename = nullptr;
			io.KeyMap[ImGuiKey_Tab] = (uint32_t)Key::Key_Tab;
			io.KeyMap[ImGuiKey_LeftArrow] = (uint32_t)Key::Key_Left;
			io.KeyMap[ImGuiKey_RightArrow] = (uint32_t)Key::Key_Right;
			io.KeyMap[ImGuiKey_UpArrow] = (uint32_t)Key::Key_Up;
			io.KeyMap[ImGuiKey_DownArrow] = (uint32_t)Key::Key_Down;
			io.KeyMap[ImGuiKey_PageUp] = (uint32_t)Key::Key_PageUp;
			io.KeyMap[ImGuiKey_PageDown] = (uint32_t)Key::Key_PageDown;
			io.KeyMap[ImGuiKey_Home] = (uint32_t)Key::Key_Home;
			io.KeyMap[ImGuiKey_End] = (uint32_t)Key::Key_End;
			io.KeyMap[ImGuiKey_Delete] = (uint32_t)Key::Key_Delete;
			io.KeyMap[ImGuiKey_Backspace] = (uint32_t)Key::Key_Backspace;
			io.KeyMap[ImGuiKey_Enter] = (uint32_t)Key::Key_Enter;
			io.KeyMap[ImGuiKey_Escape] = (uint32_t)Key::Key_Escape;
			io.KeyMap[ImGuiKey_A] = (uint32_t)Key::Key_A;
			io.KeyMap[ImGuiKey_C] = (uint32_t)Key::Key_C;
			io.KeyMap[ImGuiKey_V] = (uint32_t)Key::Key_V;
			io.KeyMap[ImGuiKey_X] = (uint32_t)Key::Key_X;
			io.KeyMap[ImGuiKey_Y] = (uint32_t)Key::Key_Y;
			io.KeyMap[ImGuiKey_Z] = (uint32_t)Key::Key_Z;

			io.Fonts->AddFontDefault();

			// Build texture atlas
			unsigned char *pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

			// Upload texture to graphics system
			// @MOVE: Call the renderer instead!
			GLint last_texture;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGenTextures(1, &imGuiState.fontTextureId);
			glBindTexture(GL_TEXTURE_2D, imGuiState.fontTextureId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			// Store our identifier
			io.Fonts->TexID = (void *)(intptr_t)imGuiState.fontTextureId;

			// Restore state
			glBindTexture(GL_TEXTURE_2D, last_texture);
		}

		static void ReleaseImGUI() {
			if (imGuiState.fontTextureId) {
				glDeleteTextures(1, &imGuiState.fontTextureId);
				ImGui::GetIO().Fonts->TexID = 0;
				imGuiState.fontTextureId = 0;
			}
		}

		static void ImGUIKeyEvent(uint64_t keyCode, Key mappedKey, bool down) {
			ImGuiIO& io = ImGui::GetIO();
			if (mappedKey != Key::Key_None) {
				io.KeysDown[(uint32_t)mappedKey] = down;
			} else {
				io.KeysDown[keyCode] = down;
			}
			io.KeyCtrl = io.KeysDown[(uint32_t)Key::Key_LeftControl] || io.KeysDown[(uint32_t)Key::Key_RightControl];
			io.KeyShift = io.KeysDown[(uint32_t)Key::Key_LeftShift] || io.KeysDown[(uint32_t)Key::Key_RightShift];
			io.KeyAlt = io.KeysDown[(uint32_t)Key::Key_LeftAlt] || io.KeysDown[(uint32_t)Key::Key_RightAlt];
			io.KeySuper = io.KeysDown[(uint32_t)Key::Key_LeftWin] || io.KeysDown[(uint32_t)Key::Key_RightWin];
		}
	#endif

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
							case KeyboardEventType::Char:
							{
							#if FS_ENABLE_IMGUI
								ImGuiIO& io = ImGui::GetIO();
								if (event.keyboard.keyCode > 0 && event.keyboard.keyCode < 0x10000) {
									io.AddInputCharacter(ImWchar(event.keyboard.keyCode));
								}
							#endif
							} break;
							case KeyboardEventType::KeyDown:
							case KeyboardEventType::KeyUp:
							{
								b32 isDown = (event.keyboard.type == KeyboardEventType::KeyDown) ? 1 : 0;
							#if FS_ENABLE_IMGUI
								ImGUIKeyEvent(event.keyboard.keyCode, event.keyboard.mappedKey, isDown > 0);
							#endif
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

		extern void RunGame(BaseGame *game) {
			InitSettings platformSettings = InitSettings();
			platformSettings.window.windowWidth = game->GetInitialWidth();
			platformSettings.window.windowHeight = game->GetInitialHeight();
			const char *title = game->GetTitle();
			if (title != nullptr) {
				strings::CopyAnsiString(title, strings::GetAnsiStringLength(title), platformSettings.window.windowTitle, (u32)utils::ArrayCount(platformSettings.window.windowTitle));
			}
			platformSettings.video.isVSync = false;
			if (InitPlatform(InitFlags::VideoOpenGL, platformSettings)) {
				Renderer *renderer = (Renderer *)new OpenGLRenderer();

			#if FS_ENABLE_IMGUI
				InitImGUI();
			#endif

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

				#if FS_ENABLE_IMGUI
					//
					// Update UI
					//
					{
						ImGuiIO& io = ImGui::GetIO();
						WindowSize windowArea = GetWindowArea();
						io.DeltaTime = TargetDeltaTime;
						io.DisplaySize.x = (float)windowArea.width;
						io.DisplaySize.y = (float)windowArea.height;
						io.DisplayFramebufferScale = ImVec2(1, 1);

						io.MousePos = ImVec2((float)currentInput->mouse.pos.x, (float)currentInput->mouse.pos.y);
						for (int mouseButton = 0; mouseButton < 3; ++mouseButton) {
							io.MouseDown[mouseButton] = currentInput->mouse.buttons[mouseButton].isDown > 0;
						}
						io.MouseWheel = currentInput->mouse.wheelDelta > 0.0f ? 1.0f : -1.0f;

						ImGui::NewFrame();
					}
				#endif

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
						game->Render(*currentInput);

					#if FS_ENABLE_IMGUI
						// Render UI
						glViewport(0, 0, windowArea.width, windowArea.height);
						ImGui::Render();
					#endif

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
			#if FS_ENABLE_IMGUI
				ReleaseImGUI();
			#endif
				delete renderer;
				ReleasePlatform();
			}
		}
	}
}
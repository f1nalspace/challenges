#include "game.h"

#include <final_platform_layer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "final_utils.h"
#include "final_renderer.h"

using namespace fpl;
using namespace fpl::window;

namespace finalspace {
	inline namespace games {
		namespace mygame {

			struct WallSide {
				f32 plane;
				// @NOTE: Relative position in minkowski space
				f32 relX;
				f32 relY;
				// @NOTE: Delta movement
				f32 deltaX;
				f32 deltaY;
				// @NOTE: Min/Max corners
				f32 minY;
				f32 maxY;
				// @NOTE: Surface normal
				Vec2f normal;
			};

			// @Temporary: I dont want to unload textures manually, this should happen automatically
			static void ReleaseTexture(Renderer &renderer, Texture &texture) {
				GLuint textureId = utils::PointerToValue<GLuint>(texture.handle);
				if (textureId) {
					glDeleteTextures(1, &textureId);
					texture = {};
				}
			}

			// @Temporary: I dont want to load textures manually, this should happen automatically when i want to access the texture (Think this trough, but for now we load it manually.)
			static Texture LoadTexture(Renderer &renderer, const char *imageFilePath) {
				Texture result = {};
				fpl::files::FileHandle imageFileHandle = fpl::files::OpenBinaryFile(imageFilePath);
				if (imageFileHandle.isValid) {
					u32 imageFileSize = fpl::files::GetFileSize32(imageFileHandle);
					if (imageFileSize) {
						u8 *imageFileData = new u8[imageFileSize];
						fpl::files::ReadFileBlock32(imageFileHandle, imageFileSize, imageFileData, imageFileSize);
						int imageWidth, imageHeight, imageComponents;
						stbi_set_flip_vertically_on_load(1);
						auto imageData = stbi_load_from_memory(imageFileData, imageFileSize, &imageWidth, &imageHeight, &imageComponents, 4);
						delete[] imageFileData;
						if (imageData != nullptr) {
							result.handle = renderer.AllocateTexture(imageWidth, imageHeight, imageData);
							result.width = imageWidth;
							result.height = imageHeight;
							stbi_image_free(imageData);
						}
					}
					fpl::files::CloseFile(imageFileHandle);
				}
				return(result);
			}

			void Game::Release() {
				controlledPlayers.clear();
				players.clear();
				walls.clear();

				// @Temporary: Remove this later, the renderer should take care of that automatically
				ReleaseTexture(renderer, tilesetTexture);
			}

			void Game::Init() {
				fpl::window::SetWindowTitle("GameDev Challenge Oct 2017");

				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

				// Load textures
				tilesetTexture = LoadTexture(renderer, "tileset.png");

				// Load test level
				constexpr char *testLevel = "test.map";
				if (LoadMap(testLevel)) {
					CreateWallsFromTiles();
					activeEditorFilePath = testLevel;
				}

			}

			u32 Game::CreatePlayer(const u32 controllerIndex) {
				Entity player = Entity();
				player.position = Vec2f();
				player.ext = Vec2f(0.4f, 0.4f);
				player.horizontalSpeed = 20.0f;
				player.horizontalDrag = 13.0f;
				player.canJump = true;
				player.jumpPower = 140.0f;

				u32 result = (u32)players.size();
				players.emplace_back(player);
				return(result);
			}

			s32 Game::FindControlledPlayerIndex(const u32 controllerIndex) {
				s32 result = -1;
				for (u32 controlledPlayerIndex = 0; controlledPlayerIndex < controlledPlayers.size(); ++controlledPlayerIndex) {
					if (controlledPlayers[controlledPlayerIndex].controllerIndex == controllerIndex) {
						result = controlledPlayerIndex;
						break;
					}
				}
				return(result);
			}

			void Game::HandleControllerConnections(const Input &input) {
				for (u32 controllerIndex = 0; controllerIndex < utils::ArrayCount(input.controllers); ++controllerIndex) {
					const Controller &testController = input.controllers[controllerIndex];
					if (testController.isConnected) {
						// @NOTE: Connected controller
						s32 foundControlledPlayerIndex = FindControlledPlayerIndex(controllerIndex);
						if (foundControlledPlayerIndex == -1) {
							u32 playerIndex;
							if (isSinglePlayer) {
								if (players.size() == 0) {
									playerIndex = CreatePlayer(controllerIndex);
								} else {
									playerIndex = 0;
								}
							} else {
								// @TODO: Limit the number of players
								playerIndex = CreatePlayer(controllerIndex);
							}
							ControlledPlayer controlledPlayer = {};
							controlledPlayer.controllerIndex = controllerIndex;
							controlledPlayer.playerIndex = playerIndex;
							controlledPlayers.emplace_back(controlledPlayer);
						}
					} else {
						// @NOTE: Disconnected controller
						s32 foundControlledPlayerIndex = FindControlledPlayerIndex(controllerIndex);
						if (foundControlledPlayerIndex != -1) {
							const ControlledPlayer &controlledPlayer = controlledPlayers[foundControlledPlayerIndex];
							u32 playerIndex = controlledPlayer.playerIndex;
							u32 controllerIndex = controlledPlayer.controllerIndex;

							// @TODO: Give the player a bit time to reconnect - let it blink or something

							// Remove player and controlled player
							players.erase(players.begin() + playerIndex);
							controlledPlayers.erase(controlledPlayers.begin() + foundControlledPlayerIndex);
						}
					}
				}
			}

			void Game::ProcessPlayerInput(const Input &input) {
				// Player forces
				for (u32 controlledPlayerIndex = 0; controlledPlayerIndex < controlledPlayers.size(); ++controlledPlayerIndex) {
					u32 playerIndex = controlledPlayers[controlledPlayerIndex].playerIndex;
					u32 controllerIndex = controlledPlayers[controlledPlayerIndex].controllerIndex;

					// @BUG: On app shutdown this crashes due to the assert below.
					// Players got cleared but connected controllers arent,
					// because the controller is not disconnected when shutting down.
					// This condition ensures that it wont crash, but its not correct!
					if (playerIndex >= (players.size())) continue;

					assert(playerIndex < players.size());
					assert(controllerIndex < utils::ArrayCount(input.controllers));

					Entity &player = players[playerIndex];
					const Controller &playerController = input.controllers[controllerIndex];

					b32 wasGrounded = player.isGrounded;

					// Set acceleration based on player input
					if (!playerController.isAnalog) {
						if (playerController.moveLeft.isDown) {
							player.acceleration.x += -1.0f * player.horizontalSpeed;
						} else if (playerController.moveRight.isDown) {
							player.acceleration.x += 1.0f * player.horizontalSpeed;
						}
					} else {
						if (playerController.analogMovement.x != 0) {
							player.acceleration.x += playerController.analogMovement.x * player.horizontalSpeed;
						}
					}

					// Jump
					if (playerController.actionDown.isDown) {
						if (wasGrounded && player.canJump && player.jumpCount == 0) {
							player.acceleration.y += 1.0f * player.jumpPower;
							++player.jumpCount;
						}
					}
				}
			}

			void Game::MovePlayers(const Input &input) {
				for (s32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
					Entity &player = players[playerIndex];

					player.isGrounded = false;

					// Movement equation:
					// p' = (a / 2) * dt^2 + v * dt + p
					// v' = a * dt + v
					Vec2f playerDelta = 0.5f * player.acceleration * (input.deltaTime * input.deltaTime) + player.velocity * input.deltaTime;
					player.velocity = player.acceleration * input.deltaTime + player.velocity;
					player.acceleration = Vec2f();

					// Do line segment tests for each wall, find the side of the wall which is nearest in time
					// To support colliding with multiple walls we iterate a few times
					for (u32 iteration = 0; iteration < 4; ++iteration) {
						f32 tmin = 1.0f;
						f32 tmax = 1.0f;

						f32 playerDeltaLen = Length(playerDelta);
						if (playerDeltaLen > 0.0f) {

							Vec2f wallNormalMin = Vec2f();
							Wall *hitWallMin = nullptr;

							Vec2f targetPosition = player.position + playerDelta;

							for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
								Wall &wall = walls[wallIndex];

								Vec2f minkowskiExt = { player.ext.x + wall.ext.x, player.ext.y + wall.ext.y };
								Vec2f minCorner = -minkowskiExt;
								Vec2f maxCorner = minkowskiExt;

								Vec2f rel = player.position - wall.position;

								WallSide testSides[4] =
								{
									{ minCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y,{ -1, 0 } },
									{ maxCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y,{ 1, 0 } },
									{ minCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x,{ 0, -1 } },
									{ maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x,{ 0, 1 } },
								};

								u32 wallSideCount = 4;

								if (wall.isPlatform) {
									// @NOTE: One a platform we just have to test for the upper side.
									wallSideCount = 1;
									testSides[0] = { maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x,{ 0, 1 } };
								}

								// @TODO: It works but i would prefered a generic line segment intersection test here
								Vec2f testSideNormal = Vec2f();
								f32 hitTime = tmin;
								bool wasHit = false;
								for (u64 testSideIndex = 0; testSideIndex < wallSideCount; ++testSideIndex) {
									WallSide *testSide = testSides + testSideIndex;
									if (testSide->deltaX != 0.0f) {
										f32 f = (testSide->plane - testSide->relX) / testSide->deltaX;
										f32 y = testSide->relY + f*testSide->deltaY;
										if ((f >= 0.0f) && (hitTime > f)) {
											if ((y >= testSide->minY) && (y <= testSide->maxY)) {
												constexpr f32 EpsilonTime = 0.001f;
												hitTime = Maximum(0.0f, f - EpsilonTime);
												testSideNormal = testSide->normal;
												wasHit = true;
											}
										}
									}
								}

								if (wasHit) {
									// Solid block or one sided platform
									if ((!wall.isPlatform) || (wall.isPlatform && (Dot(playerDelta, Vec2f::Up) <= 0))) {
										tmin = hitTime;
										wallNormalMin = testSideNormal;
										hitWallMin = &wall;
									}
								}
							}

							Vec2f wallNormal = Vec2f();
							Wall *hitWall = nullptr;
							f32 stopTime;
							if (tmin < tmax) {
								stopTime = tmin;
								hitWall = hitWallMin;
								wallNormal = wallNormalMin;
							} else {
								stopTime = tmax;
							}

							player.position += stopTime * playerDelta;

							if (hitWall != nullptr) {
								// Recalculate player delta and apply bounce (canceling out the velocity along the normal)
								f32 restitution = 0.0f;
								playerDelta = targetPosition - player.position;
								playerDelta += -(1 + restitution) * Dot(playerDelta, wallNormal) * wallNormal;
								player.velocity += -(1 + restitution) * Dot(player.velocity, wallNormal) * wallNormal;

								// Update grounded states
								if (Dot(wallNormal, Vec2f::Up) > 0) {
									player.isGrounded = true;
									player.jumpCount = 0;
								}
							}
						}
					}
				}
			}

			void Game::SetExternalForces() {
				// External forces (Gravity, drag, etc.)
				for (s32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
					Entity &player = players[playerIndex];
					player.acceleration = Vec2f();

					// Gravity
					player.acceleration += gravity;

					// Horizontal drag
					player.acceleration += -Dot(Vec2f::Right, player.velocity) * Vec2f::Right * player.horizontalDrag;
				}
			}

			void Game::EditorUpdate() {
				auto io = ImGui::GetIO();
				auto ctx = ImGui::GetCurrentContext();
				auto editorWindowSize = ImVec2(io.DisplaySize.x, io.DisplaySize.y);

				ImGui::SetNextWindowSize(editorWindowSize, ImGuiCond_Always);
				ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
				const char *mapName = activeEditorFilePath.empty() ? "Unnamed map" : activeEditorFilePath.c_str();

				ImGui::Begin(mapName, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

				// Menu
				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("New map")) {
							ClearLevel();
							activeEditorFilePath = "";
						}
						if (ImGui::MenuItem("Load map...")) {
							showOpenDialog = true;
							firstTimeOpenDialog = true;
						}
						if (ImGui::MenuItem("Save map")) {
							showSaveDialog = activeEditorFilePath.empty();
							if (!showSaveDialog)
								SaveMap(activeEditorFilePath.c_str());
							else
								firstTimeSaveDialog = true;
						}
						if (ImGui::MenuItem("Save map as...")) {
							showSaveDialog = firstTimeSaveDialog = true;
						}
						if (ImGui::MenuItem("Exit")) {
							exitRequested = true;
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				// Canvas area
				ImVec2 minRegion = ImGui::GetWindowContentRegionMin();
				ImVec2 maxRegion = ImGui::GetWindowContentRegionMax();

				// @NOTE: Y max and min are swaped, because our coordinate system is down to up, imgui is up to down
				Vec2f canvasMin = Vec2f(minRegion.x, maxRegion.y);
				Vec2f canvasMax = Vec2f(maxRegion.x, minRegion.y);

				RenderArea canvasArea = CalculateRenderArea(-Vec2f(HalfGameWidth, HalfGameHeight), Vec2f(HalfGameWidth, HalfGameHeight), canvasMin, canvasMax, true);

				ImVec2 actualCanvasMin = ImVec2(canvasArea.targetMin.x, canvasArea.targetMin.y);
				ImVec2 actualCanvasMax = ImVec2(canvasArea.targetMax.x, canvasArea.targetMax.y);

				ImDrawList* draw_list = ImGui::GetWindowDrawList();

				draw_list->AddRect(actualCanvasMin, actualCanvasMax, ImColor(255, 255, 0));

				// Draw tiles
				for (u32 y = 0; y < TileCountForHeight; ++y) {
					for (u32 x = 0; x < TileCountForWidth; ++x) {
						const Tile &tile = tiles[y * TileCountForWidth + x];
						if (tile.type != TileType::None) {
							Vec4f platformColor;
							if (tile.type == TileType::Platform)
								platformColor = Vec4f(0.0f, 0.0f, 0.75f);
							else
								platformColor = Vec4f(0.0f, 0.0f, 1.0f);

							s32 tileTypeIndex = (s32)tile.type;
							Vec2f uvMax = TileUVs[tileTypeIndex * 2 + 0];
							Vec2f uvMin = TileUVs[tileTypeIndex * 2 + 1];

							Vec2f tilePos = TileToWorld(x, y) - Vec2f(TileSize * 0.5f);
							Vec2f a = canvasArea.Project(tilePos);
							Vec2f b = canvasArea.Project(tilePos + Vec2f(TileSize));
							ImTextureID texId = tilesetTexture.handle;
							draw_list->AddImage(texId, ImVec2(a.x, a.y), ImVec2(b.x, b.y), ImVec2(uvMin.x, uvMin.y), ImVec2(uvMax.x, uvMax.y));
						}
					}
				}

				// Draw players
				for (u32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
					const Entity &player = players[playerIndex];
					Vec4f playerColor = Vec4f(1.0f, 1.0f, 1.0f);
					Vec2f playerPos = player.position - player.ext;
					Vec2f a = canvasArea.Project(playerPos);
					Vec2f b = canvasArea.Project(playerPos + player.ext * 2.0f);
					draw_list->AddRectFilled(ImVec2(a.x, a.y), ImVec2(b.x, b.y), ImColor(playerColor.r, playerColor.g, playerColor.b, playerColor.a));
				}

				// Mouse hover and click actions
				if (ImGui::IsWindowFocused() && ImGui::IsMouseHoveringRect(actualCanvasMin, actualCanvasMax)) {
					ImVec2 mp = ImGui::GetMousePos();
					Vec2f p = canvasArea.Unproject(Vec2f(mp.x, mp.y));
					Vec2i hoverTile = WorldToTile(p);
					Vec2f tilePos = TileToWorld(hoverTile) - Vec2f(TileSize) * 0.5f;
					Vec2f a = canvasArea.Project(tilePos);
					Vec2f b = canvasArea.Project(tilePos + Vec2f(TileSize));
					draw_list->AddRect(ImVec2(a.x, a.y), ImVec2(b.x, b.y), ImColor(255, 255, 0));

					if (ImGui::IsMouseDown(0)) {
						SetTile(hoverTile.x, hoverTile.y, TileType::Block);
					} else if (ImGui::IsMouseDown(2)) {
						SetTile(hoverTile.x, hoverTile.y, TileType::None);
					}
				}

				ImGui::End();

				// Save map dialog
				{
					const char *popupId = "Save map";
					if (showSaveDialog) {
						static char mapNameBuffer[1024] = {};
						static bool autoOk = false;
						if (firstTimeSaveDialog) {
							memory::ClearMemory(mapNameBuffer, sizeof(mapNameBuffer));
							strings::CopyAnsiString(activeEditorFilePath.c_str(), (u32)activeEditorFilePath.size(), mapNameBuffer, (u32)utils::ArrayCount(mapNameBuffer));
							firstTimeSaveDialog = false;
							autoOk = false;
						}
						ImGui::OpenPopup(popupId);
						if (ImGui::BeginPopupModal(popupId, &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize))
						{
							if (ImGui::InputText("Name", mapNameBuffer, IM_ARRAYSIZE(mapNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								autoOk = true;
							}
							if (ImGui::Button("OK", ImVec2(120, 0)) || autoOk) {
								ImGui::CloseCurrentPopup();
								showSaveDialog = autoOk = false;

								activeEditorFilePath = mapNameBuffer;
								activeEditorFilePath = paths::ChangeFileExtension(mapNameBuffer, (u32)utils::ArrayCount(mapNameBuffer), activeEditorFilePath.c_str(), ".map");
								SaveMap(activeEditorFilePath.c_str());
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) {
								ImGui::CloseCurrentPopup();
								showSaveDialog = autoOk = false;
							}
							ImGui::EndPopup();
						}
					}
				}

				// Load map dialog
				{
					const char *loadMapPopupId = "Load map";
					if (showOpenDialog) {
						static char loadMapNameBuffer[1024] = {};
						static bool loadMapAuto = false;
						if (firstTimeOpenDialog) {
							memory::ClearMemory(loadMapNameBuffer, sizeof(loadMapNameBuffer));
							strings::CopyAnsiString(activeEditorFilePath.c_str(), (u32)activeEditorFilePath.size(), loadMapNameBuffer, (u32)utils::ArrayCount(loadMapNameBuffer));
							firstTimeOpenDialog = false;
							loadMapAuto = false;
						}
						ImGui::OpenPopup(loadMapPopupId);
						if (ImGui::BeginPopupModal(loadMapPopupId, &showOpenDialog, ImGuiWindowFlags_AlwaysAutoResize))
						{
							if (ImGui::InputText("Name", loadMapNameBuffer, IM_ARRAYSIZE(loadMapNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								loadMapAuto = true;
							}
							if (ImGui::Button("OK", ImVec2(120, 0)) || loadMapAuto) {
								ImGui::CloseCurrentPopup();
								showOpenDialog = loadMapAuto = false;

								std::string tempFilePath = loadMapNameBuffer;
								char *tempFilePathWithExt = paths::ChangeFileExtension(loadMapNameBuffer, (u32)utils::ArrayCount(loadMapNameBuffer), tempFilePath.c_str(), ".map");
								if (LoadMap(tempFilePathWithExt)) {
									activeEditorFilePath = tempFilePathWithExt;
								}
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) {
								ImGui::CloseCurrentPopup();
								showOpenDialog = loadMapAuto = false;
							}
							ImGui::EndPopup();
						}
					}
				}


			}

			void Game::ClearLevel() {
				for (u32 tileIndex = 0; tileIndex < tiles.size(); ++tileIndex)
					tiles[tileIndex] = {};
				players.clear();
				walls.clear();
				controlledPlayers.clear();
			}
			bool Game::LoadMap(const char *filePath) {
				bool result = false;

				auto fileHandle = files::OpenBinaryFile(filePath);
				if (fileHandle.isValid) {
					u32 read;
					char magic[4] = {};
					assert(sizeof(MapMagicId) == sizeof(magic));
					read = files::ReadFileBlock32(fileHandle, sizeof(magic), &magic, sizeof(magic));
					assert(read == sizeof(magic) && (strncmp(MapMagicId, magic, utils::ArrayCount(MapMagicId)) == 0));

					ClearLevel();

					u32 tileCount = 0;
					const u32 maxTileCount = TileCountForWidth * TileCountForHeight;
					read = files::ReadFileBlock32(fileHandle, sizeof(tileCount), &tileCount, sizeof(tileCount));
					assert(read == sizeof(tileCount) && tileCount == maxTileCount);

					Tile *firstTile = &tiles[0];
					read = files::ReadFileBlock32(fileHandle, sizeof(Tile) * tileCount, firstTile, sizeof(Tile) * maxTileCount);
					assert(read == sizeof(Tile) * maxTileCount);

					result = true;

					files::CloseFile(fileHandle);
				}

				return(result);
			}
			void Game::SaveMap(const char *filePath) {
				auto fileHandle = files::CreateBinaryFile(filePath);
				if (fileHandle.isValid) {
					files::WriteFileBlock32(fileHandle, (void *)&MapMagicId, sizeof(MapMagicId));
					u32 tileCount = TileCountForWidth * TileCountForHeight;
					files::WriteFileBlock32(fileHandle, &tileCount, sizeof(tileCount));
					for (u32 tileY = 0; tileY < TileCountForHeight; ++tileY) {
						for (u32 tileX = 0; tileX < TileCountForWidth; ++tileX) {
							Tile tile = tiles[tileY * TileCountForWidth + tileX];
							files::WriteFileBlock32(fileHandle, &tile, sizeof(tile));
						}
					}
					files::CloseFile(fileHandle);
				}
			}

			void Game::CreateWallsFromTiles() {
				walls.clear();
				for (u32 y = 0; y < TileCountForHeight; ++y) {
					for (u32 x = 0; x < TileCountForWidth; ++x) {
						const Tile &tile = tiles[y * TileCountForWidth + x];
						if (tile.type != TileType::None) {
							Vec2f tileWorldPos = TileToWorld(x, y);
							Wall wall = {};
							wall.position = tileWorldPos;
							wall.ext = TileSize * 0.5f;
							wall.isPlatform = tile.type == TileType::Platform;
							walls.emplace_back(wall);
						}
					}
				}
			}

			void Game::UISaveMap(const bool withDialog) {
				if (withDialog) {
				}
			}

			void Game::HandleInput(const Input &input) {
				if (isEditor) {
					EditorUpdate();
				}

				renderer.Update(HalfGameWidth, HalfGameHeight, GameAspect);

				// Update world mouse position
				const s32 mouseX = input.mouse.pos.x;
				const s32 mouseY = renderer.windowSize.h - 1 - input.mouse.pos.y;
				mouseWorldPos = renderer.Unproject(Vec2i(mouseX, mouseY));

				if (input.keyboard.editorToggle.WasPressed()) {
					if (isEditor) {
						CreateWallsFromTiles();
					}
					isEditor = !isEditor;
				}

				if (!isEditor) {
					HandleControllerConnections(input);
				}
			}

			void Game::Update(const Input &input) {
				if (!isEditor) {
					SetExternalForces();
					ProcessPlayerInput(input);
					MovePlayers(input);
				} else {

				}
			}

			void Game::Render() {
				renderer.BeginFrame();

				const Vec2f tileExt = Vec2f(TileSize * 0.5f);

				if (!isEditor) {
					// Draw walls
					for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
						const Wall &wall = walls[wallIndex];
						s32 tileTypeIndex = (s32)TileType::Block;
						if (wall.isPlatform) {
							tileTypeIndex = (s32)TileType::Platform;
						}
						Vec2f uvMax = TileUVs[tileTypeIndex * 2 + 0];
						Vec2f uvMin = TileUVs[tileTypeIndex * 2 + 1];
						renderer.DrawSprite(wall.position, wall.ext, Vec4f::White, tilesetTexture, uvMin, uvMax);
					}

					// Draw players
					for (u32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
						const Entity &player = players[playerIndex];
						Vec4f playerColor = Vec4f(1.0f, 1.0f, 1.0f);
						renderer.DrawRectangle(player.position, player.ext, playerColor);
					}
				}

				renderer.EndFrame();
			}

			Game::Game(Renderer &renderer) : BaseGame(renderer) {
			}

			Game::~Game() {
			}
		}
	}
}
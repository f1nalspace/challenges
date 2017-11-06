#include "game.h"

#include <final_platform_layer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "final_utils.h"
#include "final_mem.h"

using namespace fpl;
using namespace fpl::window;

namespace fs {
	namespace games {
		namespace mygame {

			// @Temporary: I dont want to unload textures manually, this should happen automatically
			static void ReleaseTexture(Renderer *renderer, Texture &texture) {
				GLuint textureId = utils::PointerToValue<GLuint>(texture.handle);
				if (textureId) {
					glDeleteTextures(1, &textureId);
					texture = {};
				}
			}

			// @Temporary: I dont want to load textures manually, this should happen automatically when i want to access the texture (Think this trough, but for now we load it manually.)
			static Texture LoadTexture(Renderer *renderer, const char *imageFilePath) {
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
							result.handle = renderer->AllocateTexture(imageWidth, imageHeight, imageData);
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
				assert(renderer != nullptr);

				fpl::window::SetWindowTitle("GameDev Challenge Oct 2017");

				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


				// Load textures
				tilesetTexture = LoadTexture(renderer, "tileset.png");

				// Load test level
				constexpr char *testLevel = "test.map";
				if (LoadMap(testLevel)) {
					Reload();
					activeEditorFilePath = testLevel;
				}

			}

			ResultTilePosition Game::FindFreePlayerTile() {
				ResultTilePosition result = {};

				for (u32 tileY = 0; tileY < TILE_COUNT_FOR_HEIGHT; ++tileY) {
					for (u32 tileX = 0; tileX < TILE_COUNT_FOR_WIDTH; ++tileX) {
						const Tile &tile = GetTile(tileX, tileY);
						if (tile.type == TileType::Player) {
							// @TODO: Find tile where is most far away from existing entities
							result.found = true;
							result.tileX = tileX;
							result.tileY = tileY;
							break;
						}
					}
				}

				return(result);
			}

			static void NextAIDecision(Entity &enemy, f32 duration, AIState::Type nextType) {
				enemy.ai.waitRemaingTime = duration;
				enemy.ai.isWaiting = true;
				enemy.ai.nextType = nextType;
			}

			EnemyIndex Game::CreateEnemy(u32 tileX, u32 tileY) {
				Vec2f enemyCenterOnTile = TileToWorld(tileX, tileY);

				Entity enemy = Entity();
				enemy.ext = Vec2f(0.3f, 0.3f);
				enemy.position = enemyCenterOnTile - Vec2f(0, TILE_SIZE * 0.5f) + Vec2f(0, enemy.ext.y + EntityPlaceOffset);
				enemy.type = Entity::Type::Enemy;
				enemy.color = Vec4f(1.0f, 0.0f, 1.0f);
				enemy.horizontalSpeed = 2.0f;
				enemy.horizontalDrag = 4.0f;
				enemy.canJump = true;
				enemy.jumpPower = 120.0f;
				NextAIDecision(enemy, 1.0f, AIState::Type::DecideDirection);

				u32 result = (u32)enemies.size();
				enemies.emplace_back(enemy);
				return(result);
			}

			PlayerIndex Game::CreatePlayer(const u32 controllerIndex) {
				ResultTilePosition resultPos = FindFreePlayerTile();
				assert(resultPos.found);
				Vec2f playerCenterOnTile = TileToWorld(resultPos.tileX, resultPos.tileY);

				Entity player = Entity();
				player.type = Entity::Type::Player;
				player.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
				player.ext = Vec2f(0.4f, 0.4f);
				player.position = playerCenterOnTile - Vec2f(0, TILE_SIZE * 0.5f) + Vec2f(0, player.ext.y + EntityPlaceOffset);
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

			static bool CanJump(Entity &entity) {
				bool result = (entity.isGrounded && entity.canJump && entity.jumpCount == 0);
				return result;
			}

			static void Jump(Entity &entity) {
				if (CanJump(entity)) {
					entity.acceleration.y += 1.0f * entity.jumpPower;
					entity.moveDirection.y = 1;
					++entity.jumpCount;
				}
			}

			void Game::ProcessEnemyAI(const f32 deltaTime) {
				constexpr f32 DefaultCoolDown = 0.5f;
				constexpr f32 JumpCoolDown = 0.05f;
				constexpr f32 ReflectCoolDown = 0.01f;
				constexpr f32 ToMoveCoolDown = 0.1f;

				for (u32 enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex) {
					Entity &enemy = enemies[enemyIndex];

					if (enemy.ai.isWaiting) {
						enemy.ai.waitRemaingTime -= deltaTime;
						if (enemy.ai.waitRemaingTime <= 0.0f) {
							enemy.ai.waitRemaingTime = 0.0f;
							enemy.ai.isWaiting = false;
							enemy.ai.activeType = enemy.ai.nextType;
							enemy.ai.nextType = AIState::Type::None;
							enemy.ai.activeDuration = 0.0f;
							enemy.ai.dice = RandomUnilateral(enemyEntropy);
						}
						continue;
					}

					switch (enemy.ai.activeType) {
						case AIState::Type::Move:
						{
							// Decide something when we collide
							if (enemy.ai.nextType == AIState::Type::None) {
								for (u32 collisionIndex = 0; collisionIndex < enemy.collisionCount; ++collisionIndex) {
									CollisionState *collision = enemy.collisions + collisionIndex;
									Vec2f sideProj = Dot(Vec2f::Right, collision->normal);
									if (Absolute(sideProj.x) > 0) {
										NextAIDecision(enemy, 0.0f, AIState::Type::ReflectDirection);
										break;
									}
								}
							}

							Vec2f acc = enemy.moveDirection * enemy.horizontalSpeed;
							enemy.acceleration += acc;
						} break;

						case AIState::Type::DecideDirection:
						{
							s32 dir = RandomBetweenInt(enemyEntropy, -1, 1);
							f32 directionX = (dir < 0) ? -1.0f : 1.0f;
							enemy.moveDirection.x = directionX;
							NextAIDecision(enemy, ToMoveCoolDown, AIState::Type::Move);
						} break;

						case AIState::Type::ReflectDirection:
						{
							enemy.moveDirection.x = -enemy.moveDirection.x;
							NextAIDecision(enemy, ToMoveCoolDown, AIState::Type::Move);
						} break;

						case AIState::Type::Jump:
						{
							Jump(enemy);
							NextAIDecision(enemy, 0.0f, AIState::Type::Move);
						} break;
					}

					enemy.ai.activeDuration += deltaTime;

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

					// Set acceleration based on player input
					player.moveDirection = Vec2f();
					if (!playerController.isAnalog) {
						if (playerController.moveLeft.isDown) {
							player.acceleration.x += -1.0f * player.horizontalSpeed;
							player.moveDirection.x = -1;
						} else if (playerController.moveRight.isDown) {
							player.acceleration.x += 1.0f * player.horizontalSpeed;
							player.moveDirection.x = 1;
						}
					} else {
						if (playerController.analogMovement.x != 0) {
							player.acceleration.x += playerController.analogMovement.x * player.horizontalSpeed;
						}
					}

					// Jump
					if (playerController.actionDown.isDown) {
						Jump(player);
					}
				}
			}

			void Game::MoveEntities(std::vector<Entity> &entities, const f32 deltaTime) {
				for (s32 entityIndex = 0; entityIndex < entities.size(); ++entityIndex) {
					Entity &entity = entities[entityIndex];

					entity.isGrounded = false;
					entity.collisionCount = 0;

					// Movement equation:
					// p' = (a / 2) * dt^2 + v * dt + p
					// v' = a * dt + v
					Vec2f deltaMovement = 0.5f * entity.acceleration * (deltaTime * deltaTime) + entity.velocity * deltaTime;
					entity.velocity = entity.acceleration * deltaTime + entity.velocity;
					entity.acceleration = Vec2f();

					// Do line segment tests for each wall, find the side of the wall which is nearest in time
					// To support colliding with multiple walls we iterate a few times
					for (u32 iteration = 0; iteration < 4; ++iteration) {
						f32 tmin = 1.0f;
						f32 tmax = 1.0f;

						f32 deltaMovementLen = Length(deltaMovement);
						if (deltaMovementLen > 0.0f) {

							Vec2f wallNormalMin = Vec2f();
							Wall *hitWallMin = nullptr;

							Vec2f targetPosition = entity.position + deltaMovement;

							for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
								Wall &wall = walls[wallIndex];

								Vec2f minkowskiExt = { entity.ext.x + wall.ext.x, entity.ext.y + wall.ext.y };
								Vec2f minCorner = -minkowskiExt;
								Vec2f maxCorner = minkowskiExt;

								Vec2f rel = entity.position - wall.position;

								DeltaPlane2D testSides[4];
								u32 sideCount = 0;
								testSides[sideCount++] = { minCorner.x, rel.x, rel.y, deltaMovement.x, deltaMovement.y, minCorner.y, maxCorner.y,{ -1, 0 } };
								testSides[sideCount++] = { maxCorner.x, rel.x, rel.y, deltaMovement.x, deltaMovement.y, minCorner.y, maxCorner.y,{ 1, 0 } };
								testSides[sideCount++] = { minCorner.y, rel.y, rel.x, deltaMovement.y, deltaMovement.x, minCorner.x, maxCorner.x,{ 0, -1 } };
								testSides[sideCount++] = { maxCorner.y, rel.y, rel.x, deltaMovement.y, deltaMovement.x, minCorner.x, maxCorner.x,{ 0, 1 } };

								if (wall.isPlatform) {
									// @NOTE: One a platform we just have to test for the upper side.
									sideCount = 1;
									testSides[0] = { maxCorner.y, rel.y, rel.x, deltaMovement.y, deltaMovement.x, minCorner.x, maxCorner.x,{ 0, 1 } };
								}

								// @TODO: It works but i would prefered a generic line segment intersection test here
								// Create line segments for each sides of a tile
								constexpr f32 EpsilonTime = 0.001f;
								IntersectionResult intersectionResult = IntersectLines(tmin, EpsilonTime, sideCount, testSides);
								if (intersectionResult.wasHit) {
									// Solid block or one sided platform
									if ((!wall.isPlatform) || (wall.isPlatform && (Dot(deltaMovement, Vec2f::Up) <= 0))) {
										tmin = intersectionResult.tMin;
										wallNormalMin = intersectionResult.normal;
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

							entity.position += stopTime * deltaMovement;

							if (hitWall != nullptr) {
								// Recalculate player delta and apply bounce (canceling out the velocity along the normal)
								f32 restitution = 0.0f;
								deltaMovement = targetPosition - entity.position;
								deltaMovement += -(1 + restitution) * Dot(deltaMovement, wallNormal) * wallNormal;
								entity.velocity += -(1 + restitution) * Dot(entity.velocity, wallNormal) * wallNormal;

								// Update grounded states
								if (Dot(wallNormal, Vec2f::Up) > 0) {
									entity.isGrounded = true;
									entity.jumpCount = 0;
								}

								// Add collision state
								assert(entity.collisionCount < utils::ArrayCount(entity.collisions));
								u32 collisionIndex = entity.collisionCount++;
								CollisionState *collision = &entity.collisions[collisionIndex];
								*collision = {};
								collision->isColliding = true;
								collision->wall = hitWall;
								collision->normal = wallNormal;
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

				for (s32 enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex) {
					Entity &enemy = enemies[enemyIndex];
					enemy.acceleration = Vec2f();

					// Gravity
					enemy.acceleration += gravity;

					// Horizontal drag
					enemy.acceleration += -Dot(Vec2f::Right, enemy.velocity) * Vec2f::Right * enemy.horizontalDrag;
				}
			}

			static ImRect CreateNormalizedRect(const ImVec2 &min, ImVec2 &max) {
				ImVec2 a = ImVec2(Minimum(min.x, max.x), Minimum(min.y, max.y));
				ImVec2 b = ImVec2(Maximum(min.x, max.x), Maximum(min.y, max.y));
				ImRect result = ImRect(a, b);
				return(result);
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
							ClearMap();
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

				// Tool bar
				if (ImGui::CollapsingHeader("Toolbar", nullptr, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Text("Tile:");
					ImGui::SameLine();
					if (ImGui::RadioButton("None", selectedTileType == TileType::None)) {
						selectedTileType = TileType::None;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Block", selectedTileType == TileType::Block)) {
						selectedTileType = TileType::Block;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Platform", selectedTileType == TileType::Platform)) {
						selectedTileType = TileType::Platform;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Player", selectedTileType == TileType::Player)) {
						selectedTileType = TileType::Player;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Enemy", selectedTileType == TileType::Enemy)) {
						selectedTileType = TileType::Enemy;
					}
				}

				// Canvas panel
				if (ImGui::CollapsingHeader("Canvas", nullptr, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen)) {
					ImVec2 minRegion = ImGui::GetCursorPos();
					ImVec2 contentAvailable = ImGui::GetContentRegionAvail();
					ImVec2 maxRegion = ImVec2(minRegion.x + contentAvailable.x, minRegion.y + contentAvailable.y);
					// @NOTE: Y max and min are swapped, because our coordinate system is down to up, imgui is up to down
					Vec2f canvasMin = Vec2f(minRegion.x, maxRegion.y);
					Vec2f canvasMax = Vec2f(maxRegion.x, minRegion.y);

					RenderArea canvasArea = CalculateRenderArea(-Vec2f(HALF_GAME_WIDTH, HALF_GAME_HEIGHT), Vec2f(HALF_GAME_WIDTH, HALF_GAME_HEIGHT), canvasMin, canvasMax, true);

					ImVec2 actualCanvasMin = ImVec2(canvasArea.targetMin.x, canvasArea.targetMin.y);
					ImVec2 actualCanvasMax = ImVec2(canvasArea.targetMax.x, canvasArea.targetMax.y);
					ImRect canvasRect = CreateNormalizedRect(actualCanvasMin, actualCanvasMax);

					ImDrawList* draw_list = ImGui::GetWindowDrawList();

					// Draw tiles
					for (u32 y = 0; y < TILE_COUNT_FOR_HEIGHT; ++y) {
						for (u32 x = 0; x < TILE_COUNT_FOR_WIDTH; ++x) {
							const Tile &tile = tiles[y * TILE_COUNT_FOR_WIDTH + x];
							if (tile.type != TileType::None) {
								s32 tileTypeIndex = (s32)tile.type;
								Vec2f uvMax = TileUVs[tileTypeIndex * 2 + 0];
								Vec2f uvMin = TileUVs[tileTypeIndex * 2 + 1];

								Vec2f tilePos = TileToWorld(x, y) - Vec2f(TILE_SIZE * 0.5f);
								Vec2f a = canvasArea.Project(tilePos);
								Vec2f b = canvasArea.Project(tilePos + Vec2f(TILE_SIZE));
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

					draw_list->AddRect(canvasRect.Min, canvasRect.Max, ImColor(255, 255, 0));

					// Mouse hover and click actions
					const ImVec2 mp = ImGui::GetMousePos();
					if (ImGui::IsWindowFocused() && canvasRect.Contains(mp)) {
						Vec2f p = canvasArea.Unproject(Vec2f(mp.x, mp.y));
						Vec2i hoverTile = WorldToTile(p);
						Vec2f tilePos = TileToWorld(hoverTile) - Vec2f(TILE_SIZE) * 0.5f;
						Vec2f a = canvasArea.Project(tilePos);
						Vec2f b = canvasArea.Project(tilePos + Vec2f(TILE_SIZE));

						if (selectedTileType != TileType::None) {
							// @TODO: This is the same code as drawing a tile in the loop, make a function!
							s32 tileTypeIndex = (s32)selectedTileType;
							Vec2f uvMax = TileUVs[tileTypeIndex * 2 + 0];
							Vec2f uvMin = TileUVs[tileTypeIndex * 2 + 1];
							ImTextureID texId = tilesetTexture.handle;
							ImU32 color = 0xAFFFFFFF;
							draw_list->AddImage(texId, ImVec2(a.x, a.y), ImVec2(b.x, b.y), ImVec2(uvMin.x, uvMin.y), ImVec2(uvMax.x, uvMax.y), color);
						}
						draw_list->AddRect(ImVec2(a.x, a.y), ImVec2(b.x, b.y), ImColor(255, 255, 100));

						if (ImGui::IsMouseDown(0)) {
							SetTile(hoverTile.x, hoverTile.y, selectedTileType);
						}
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
						if (ImGui::BeginPopupModal(popupId, &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
							if (ImGui::InputText("Name", mapNameBuffer, IM_ARRAYSIZE(mapNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								autoOk = true;
							}
							if (ImGui::Button("OK", ImVec2(120, 0)) || autoOk) {
								ImGui::CloseCurrentPopup();
								showSaveDialog = autoOk = false;

								activeEditorFilePath = mapNameBuffer;
								activeEditorFilePath = paths::ChangeFileExtension(activeEditorFilePath.c_str(), ".map", mapNameBuffer, (u32)utils::ArrayCount(mapNameBuffer));
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
						if (ImGui::BeginPopupModal(loadMapPopupId, &showOpenDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
							if (ImGui::InputText("Name", loadMapNameBuffer, IM_ARRAYSIZE(loadMapNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								loadMapAuto = true;
							}
							if (ImGui::Button("OK", ImVec2(120, 0)) || loadMapAuto) {
								ImGui::CloseCurrentPopup();
								showOpenDialog = loadMapAuto = false;

								std::string tempFilePath = loadMapNameBuffer;
								char *tempFilePathWithExt = paths::ChangeFileExtension(tempFilePath.c_str(), ".map", loadMapNameBuffer, (u32)utils::ArrayCount(loadMapNameBuffer));
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

			void Game::ClearMap() {
				for (u32 tileIndex = 0; tileIndex < tiles.size(); ++tileIndex)
					tiles[tileIndex] = {};
				players.clear();
				enemies.clear();
				walls.clear();
				controlledPlayers.clear();
			}
			bool Game::LoadMap(const char *filePath) {
				bool result = false;

				auto fileHandle = files::OpenBinaryFile(filePath);
				if (fileHandle.isValid) {
					u32 read;
					char magic[4] = {};
					assert(sizeof(MAP_MAGIC_ID) == sizeof(magic));
					read = files::ReadFileBlock32(fileHandle, sizeof(magic), &magic, sizeof(magic));
					assert(read == sizeof(magic) && (strncmp(MAP_MAGIC_ID, magic, utils::ArrayCount(MAP_MAGIC_ID)) == 0));

					ClearMap();

					u32 tileCount = 0;
					const u32 maxTileCount = TILE_COUNT_FOR_WIDTH * TILE_COUNT_FOR_HEIGHT;
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
					files::WriteFileBlock32(fileHandle, (void *)&MAP_MAGIC_ID, sizeof(MAP_MAGIC_ID));
					u32 tileCount = TILE_COUNT_FOR_WIDTH * TILE_COUNT_FOR_HEIGHT;
					files::WriteFileBlock32(fileHandle, &tileCount, sizeof(tileCount));
					for (u32 tileY = 0; tileY < TILE_COUNT_FOR_HEIGHT; ++tileY) {
						for (u32 tileX = 0; tileX < TILE_COUNT_FOR_WIDTH; ++tileX) {
							Tile tile = tiles[tileY * TILE_COUNT_FOR_WIDTH + tileX];
							files::WriteFileBlock32(fileHandle, &tile, sizeof(tile));
						}
					}
					files::CloseFile(fileHandle);
				}
			}

			void Game::Reload() {
				enemyEntropy = RandomSeed(1337);

				// Create walls
				walls.clear();
				for (u32 y = 0; y < TILE_COUNT_FOR_HEIGHT; ++y) {
					for (u32 x = 0; x < TILE_COUNT_FOR_WIDTH; ++x) {
						const Tile &tile = GetTile(x, y);
						if (tile.type == TileType::Block || tile.type == TileType::Platform) {
							Vec2f tileWorldPos = TileToWorld(x, y);
							Wall wall = {};
							wall.position = tileWorldPos;
							wall.ext = TILE_SIZE * 0.5f;
							wall.isPlatform = tile.type == TileType::Platform;
							wall.tileType = tile.type;
							walls.emplace_back(wall);
						}
					}
				}

				// Create enemies
				enemies.clear();
				for (u32 y = 0; y < TILE_COUNT_FOR_HEIGHT; ++y) {
					for (u32 x = 0; x < TILE_COUNT_FOR_WIDTH; ++x) {
						const Tile &tile = GetTile(x, y);
						if (tile.type == TileType::Enemy) {
							CreateEnemy(x, y);
						}
					}
				}

				// Create path nodes
				enemyPath.clear();
				for (u32 y = 0; y < TILE_COUNT_FOR_HEIGHT; ++y) {
					for (u32 x = 0; x < TILE_COUNT_FOR_WIDTH; ++x) {
						if (IsSolid(x, y)) {
							if (IsValidTilePosition(x, y + 1) && IsValidTilePosition(x, y + 2)) {
								TileType typeAbove1 = GetTileType(x, y + 1);
								TileType typeAbove2 = GetTileType(x, y + 2);
								if (typeAbove1 != TileType::Platform && typeAbove1 != TileType::Block) {
									PathNode node = {};
									node.tilePosition = Vec2i(x, y + 1);
									node.worldPosition = TileToWorld(node.tilePosition.x, node.tilePosition.y);
									enemyPath.emplace_back(node);
								}
							}
						}
					}
				}

				// Compute closest nodes
				const Vec2f searchDirections[] = {
					Vec2f(0, 1), // Up
					Vec2f(-1, 1), // Left-up
					Vec2f(-1, 0), // Left
					Vec2f(-1, -1), // Left-down
					Vec2f(0, -1), // Down
					Vec2f(1, -1), // Right-Down
					Vec2f(1, 0), // Right
					Vec2f(1, 1), // Right-up
				};

				for (u32 targetNodeIndex = 0; targetNodeIndex < enemyPath.size(); ++targetNodeIndex) {
					PathNode &targetNode = enemyPath[targetNodeIndex];

					for (u32 dirIndex = 0; dirIndex < utils::ArrayCount(searchDirections); ++dirIndex) {
						const Vec2f &searchDir = searchDirections[dirIndex];

						PathNode *closestNode = nullptr;
						f32 closestDistance = F32_MAX;
						for (u32 sourceNodeIndex = 0; sourceNodeIndex < enemyPath.size(); ++sourceNodeIndex) {
							if (targetNodeIndex != sourceNodeIndex) {
								PathNode &sourceNode = enemyPath[sourceNodeIndex];
								
								Vec2f relativeDistance = sourceNode.worldPosition - targetNode.worldPosition;

								Ray2D ray = Ray2D(targetNode.worldPosition, sourceNode.worldPosition, 1.0f);

								LineCastResult lineCast = DoLineCast(ray);

								if (!lineCast.isHit) {
									f32 proj = Dot(searchDir, relativeDistance);
									if (proj > 0) {
										if ((closestNode == nullptr) || (proj < closestDistance)) {
											closestDistance = proj;
											closestNode = &sourceNode;
										}
									}
								}
							}
						}

						assert(dirIndex < utils::ArrayCount(targetNode.closestNodes));
						targetNode.closestNodes[dirIndex] = closestNode;

					}

				}
			}

			void Game::UISaveMap(const bool withDialog) {
				if (withDialog) {
				}
			}

			void Game::HandleInput(const Input &input) {
				renderer->Update(HALF_GAME_WIDTH, HALF_GAME_HEIGHT, GAME_ASPECT);

				// Update world mouse position
				const s32 mouseX = input.mouse.pos.x;
				const s32 mouseY = renderer->windowSize.h - 1 - input.mouse.pos.y;
				mouseWorldPos = renderer->Unproject(Vec2i(mouseX, mouseY));

				if (isEditor) {
					EditorUpdate();
				}

				if (input.keyboard.editorToggle.WasPressed()) {
					if (isEditor) {
						Reload();
					}
					isEditor = !isEditor;
				}

			#if !TEST_ACTIVE
				if (!isEditor) {
					HandleControllerConnections(input);
				}
			#else
			#	if TEST_RAYCASTS
				if (!isEditor) {
					if (input.mouse.left.isDown) {
						rayEnd = mouseWorldPos;
					}
				}
			#	endif
			#endif
				}

			void Game::Update(const Input &input) {
			#if !TEST_ACTIVE
				if (!isEditor) {
					SetExternalForces();
					ProcessPlayerInput(input);
					ProcessEnemyAI(input.deltaTime);
					MoveEntities(players, input.deltaTime);
					MoveEntities(enemies, input.deltaTime);
				} else {

				}
			#endif
			}

			LineCastResult Game::DoLineCast(const Ray2D &ray) {
				LineCastResult result = {};

				std::vector<Vec2i> lineTiles;
				Vec2i startTilePos = WorldToTile(ray.start);
				Vec2i endTilePos = WorldToTile(ray.end);
				BresenhamLine(startTilePos.x, startTilePos.y, endTilePos.x, endTilePos.y, 1.0f, lineTiles);

				result.tMin = 1.0f;
				for (Vec2i lineTile : lineTiles) {
					if (IsSolid(lineTile)) {
						Vec2f tilePos = TileToWorld(lineTile);
						Vec2f rel = ray.start - tilePos;

						Quad quad = {};
						quad.center = tilePos;
						quad.ext = TILE_EXT;

						LineCastResult castResult = LineCastQuad(ray, quad);
						if (castResult.isHit && castResult.tMin < result.tMin) {
							result.isHit = true;
							result.tMin = castResult.tMin;
							result.surfaceNormal = castResult.surfaceNormal;
						}
					}
				}

				return(result);
			}

			void Game::Render() {
				renderer->BeginFrame();

			#if !TEST_ACTIVE

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
						renderer->DrawSprite(wall.position, wall.ext, Vec4f::White, tilesetTexture, uvMin, uvMax);
					}

					// Draw enemies
					for (u32 enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex) {
						const Entity &enemy = enemies[enemyIndex];
						assert(enemy.type == Entity::Type::Enemy);
						renderer->DrawRectangle(enemy.position, enemy.ext, enemy.color);
					}

					// Draw players
					for (u32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
						const Entity &player = players[playerIndex];
						assert(player.type == Entity::Type::Player);
						renderer->DrawRectangle(player.position, player.ext, player.color);
					}

					// Draw path nodes
					Vec2f nodeExt = Vec2f(TILE_SIZE * 0.15);
					for (u32 nodeIndex = 0; nodeIndex < enemyPath.size(); ++nodeIndex) {
						const PathNode &node = enemyPath[nodeIndex];
						renderer->DrawRectangle(node.worldPosition, nodeExt, Vec4f::Blue);
					}
				}
			#else
			#	if TEST_RAYCASTS
				for (u32 y = 0; y < TILE_COUNT_FOR_HEIGHT; ++y) {
					for (u32 x = 0; x < TILE_COUNT_FOR_WIDTH; ++x) {
						const Tile &tile = GetTile(x, y);
						if (tile.type == TileType::Block || tile.type == TileType::Platform) {
							Vec2f p = TileToWorld(x, y);
							renderer->DrawRectangle(p, TILE_EXT, Vec4f::Blue);
						}
					}
				}

				Vec2f delta = rayEnd - rayStart;
				Vec2f rayDirection = Normalize(delta);
				f32 rayLength = Length(delta);

				renderer->DrawRectangle(rayStart, TILE_EXT * 0.25f, Vec4f::Green);
				renderer->DrawRectangle(rayEnd, TILE_EXT * 0.25f, Vec4f::Red);
				renderer->DrawLine(rayStart, rayEnd, Vec4f::White);

				std::vector<Vec2i> lineTiles;
				Vec2i rayStartTilePos = WorldToTile(rayStart);
				Vec2i rayEndTilePos = WorldToTile(rayEnd);

				int x0 = rayStartTilePos.x;
				int x1 = rayEndTilePos.x;
				int y0 = rayStartTilePos.y;
				int y1 = rayEndTilePos.y;

				BresenhamLine(x0, y0, x1, y1, 1.0f, lineTiles);

				for (const Vec2i &lineTile : lineTiles) {
					renderer->DrawRectangle(TileToWorld(lineTile), TILE_EXT, Vec4f(1.0f, 0.0f, 1.0f, 0.5f));
				}

				f32 tMin = 1.0f;
				b32 wasHit = false;
				for (Vec2i lineTile : lineTiles) {
					if (IsSolid(lineTile)) {
						Vec2f tilePos = TileToWorld(lineTile);
						Vec2f rel = rayStart - tilePos;

						Quad quad = {};
						quad.center = tilePos;
						quad.ext = TILE_EXT;

						Ray2D ray = {};
						ray.tMin = tMin;
						ray.start = rayStart;
						ray.direction = rayDirection;
						ray.length = rayLength;

						LineCastResult castResult = LineCastQuad(ray, quad);
						if (castResult.isHit && castResult.tMin < tMin) {
							wasHit = true;
							tMin = castResult.tMin;
						}
					}
				}

				if (wasHit) {
					Vec2f collisionPoint = rayStart + rayDirection * (rayLength * tMin);
					renderer->DrawRectangle(collisionPoint, TILE_EXT * 0.2f, Vec4f::Yellow);
				}

			#	else
			#		error "Invalid test!"
			#	endif
			#endif

				renderer->EndFrame();
				}

			Game::Game() : BaseGame() {
			}

			Game::~Game() {
			}
			}
			}
		}
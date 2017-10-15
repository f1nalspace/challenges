#include "game.h"

#include <final_platform_layer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "final_utils.h"
#include "final_opengl.h"

using namespace finalspace::renderer;
using namespace finalspace::utils;

namespace finalspace {
	namespace games {

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

		void Game::Release(Renderer &renderer) {
			controlledPlayers.clear();
			players.clear();
			walls.clear();

			// @Temporary: Remove this later, the renderer should take care of that automatically
			GLuint textureId = PointerToValue<GLuint>(texture.handle);
			if (textureId) {
				glDeleteTextures(1, &textureId);
				texture = {};
			}
		}

		// @Temporary: I dont want to load textures manually, this should happen automatically when i want to access the texture
		// Think this trough, but for now we load it manually.
		static Texture LoadTexture(Renderer &renderer, const char *imageFilePath) {
			Texture result = {};
			fpl::files::FileHandle imageFileHandle = fpl::files::OpenBinaryFile(imageFilePath);
			if (imageFileHandle.isValid) {
				u32 imageFileSize = fpl::files::GetFileSize32(imageFileHandle);
				if (imageFileSize) {
					u8 *imageFileData = new u8[imageFileSize];
					fpl::files::ReadFileBlock32(imageFileHandle, imageFileSize, imageFileData, imageFileSize);
					int imageWidth, imageHeight, imageComponents;
					auto imageData = stbi_load_from_memory(imageFileData, imageFileSize, &imageWidth, &imageHeight, &imageComponents, 4);
					delete[] imageFileData;
					if (imageData != nullptr) {
						result.handle = renderer.AllocateTexture(imageWidth, imageHeight, imageData);
						result.width = imageWidth;
						result.height = imageHeight;
						stbi_image_free(imageData);
					}
				}
				fpl::files::CloseFile2(imageFileHandle);
			}
			return(result);
		}

		void Game::Init(Renderer &renderer) {
			fpl::window::SetWindowTitle("GameDev Challenge Oct 2017");

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			// @Temporary: Remove when have a proper asset system
			constexpr char *imageFilePath = "brickwall.png";
			texture = LoadTexture(renderer, imageFilePath);

			// @Temporary: Fixed level for now
			{
				constexpr float WallDepth = TileSize;
				Wall wall;

				// Solid side walls
				wall = Wall();
				wall.position = Vec2f(0, -HalfGameHeight + WallDepth * 0.5f);
				wall.ext = Vec2f(HalfGameWidth, WallDepth * 0.5f);
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(0, HalfGameHeight - WallDepth * 0.5f);
				wall.ext = Vec2f(HalfGameWidth, WallDepth * 0.5f);
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(-HalfGameWidth + WallDepth * 0.5f, 0);
				wall.ext = Vec2f(WallDepth * 0.5f, HalfGameHeight - (WallDepth));
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(HalfGameWidth - WallDepth * 0.5f, 0);
				wall.ext = Vec2f(WallDepth * 0.5f, HalfGameHeight - (WallDepth));
				walls.emplace_back(wall);

				// Small solid tiles
				wall = Wall();
				wall.position = Vec2f(-HalfGameWidth + 2.0f, -3.25f);
				wall.ext = Vec2f(WallDepth * 0.5f, WallDepth * 0.5f);
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(HalfGameWidth - 2.0f, -3.25f);
				wall.ext = Vec2f(WallDepth * 0.5f, WallDepth * 0.5f);
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(2.0f, 3.25f);
				wall.ext = Vec2f(WallDepth * 0.5f, WallDepth * 0.5f);
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(-2.0f, 3.25f);
				wall.ext = Vec2f(WallDepth * 0.5f, WallDepth * 0.5f);
				walls.emplace_back(wall);

				// Center platforms
				wall = Wall();
				wall.position = Vec2f(0, -3.0f);
				wall.ext = Vec2f(4.0f, WallDepth * 0.5f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				wall = Wall();
				wall.position = Vec2f(0, 1.25f);
				wall.ext = Vec2f(4.0f, WallDepth * 0.5f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				// Side platforms
				wall = Wall();
				wall.ext = Vec2f(1.5f, WallDepth * 0.5f);
				wall.position = Vec2f(-HalfGameWidth + wall.ext.w + WallDepth, -1.0f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				wall = Wall();
				wall.ext = Vec2f(1.5f, WallDepth * 0.5f);
				wall.position = Vec2f(HalfGameWidth - wall.ext.w - WallDepth, -1.0f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				wall = Wall();
				wall.ext = Vec2f(1.0f, WallDepth * 0.5f);
				wall.position = Vec2f(0.0f, -1.0f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				wall = Wall();
				wall.ext = Vec2f(1.0f, WallDepth * 0.5f);
				wall.position = Vec2f(-HalfGameWidth + wall.ext.w + WallDepth, 3.0f);
				wall.isPlatform = true;
				walls.emplace_back(wall);

				wall = Wall();
				wall.ext = Vec2f(1.0f, WallDepth * 0.5f);
				wall.position = Vec2f(HalfGameWidth - wall.ext.w - WallDepth, 3.0f);
				wall.isPlatform = true;
				walls.emplace_back(wall);
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

		void Game::HandleControllerConnections(const finalspace::inputs::Input & input)
		{
			for (u32 controllerIndex = 0; controllerIndex < ArrayCount(input.controllers); ++controllerIndex) {
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

		void Game::ProcessPlayerInput(const finalspace::inputs::Input & input)
		{
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
				assert(controllerIndex < ArrayCount(input.controllers));

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

		void Game::MovePlayers(const finalspace::inputs::Input & input)
		{
			// @BUG: Bug in the collision system, see the comment below 
			/*
			  	There is a bug in the collision system, which prevents the player from jumping while pushing to the right or left.
				It has something todo with the ceiling or something.
				Y movement is stopped, seems the players get stuck when jumping and moving at the same time.
				     
					P[ ]
				  [ ]
				  
			*/	
			

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
							player.isGrounded = Dot(wallNormal, Vec2f::Up) > 0;
							if (player.isGrounded) {
								player.jumpCount = 0;
							}
						}
					}
				}
			}
		}

		void Game::SetExternalForces()
		{
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

		void Game::EditorUpdate(const finalspace::inputs::Input & input)
		{
			//
			// Editor input
			//
			const Vec2f tileExt = Vec2f(TileSize) * 0.5f;
			const Vec2i mouseTile = WorldToTile(mouseWorldPos);

			const bool isInsideTilemap =
				(mouseTile.x >= 0 && mouseTile.x < TileCountForWidth) &&
				(mouseTile.y >= 0 && mouseTile.y < TileCountForHeight);

			if (isInsideTilemap) {
				const u32 tileIndex = mouseTile.y * TileCountForWidth + mouseTile.x;
				assert(tileIndex < ArrayCount(tiles));
				if (input.mouse.left.isDown) {
					tiles[tileIndex].isSolid = true;
				}
				if (input.mouse.right.isDown) {
					tiles[tileIndex].isSolid = false;
				}
			}
		}

		void Game::SwitchFromEditorToGame() {
			walls.clear();

			for (u32 y = 0; y < TileCountForHeight; ++y) {
				for (u32 x = 0; x < TileCountForWidth; ++x) {
					const Tile &tile = tiles[y * TileCountForWidth + x];
					if (tile.isSolid) {
						Vec2f tileWorldPos = TileToWorld(x, y);
						Wall wall = {};
						wall.position = tileWorldPos;
						wall.ext = TileSize * 0.5f;
						walls.emplace_back(wall);
					}
				}
			}
		}

		void Game::HandleInput(Renderer &renderer, const Input &input) {
			renderer.Update(HalfGameWidth, HalfGameHeight, GameAspect);

			// Update world mouse position
			const s32 mouseX = input.mouse.pos.x;
			const s32 mouseY = renderer.windowSize.h - 1 - input.mouse.pos.y;
			mouseWorldPos = renderer.Unproject(Vec2i(mouseX, mouseY));

			if (input.keyboard.editorToggle.WasPressed()) {
				if (isEditor) {
					SwitchFromEditorToGame();
				}
				isEditor = !isEditor;
			}

			if (isEditor) {
				EditorUpdate(input);
			} else {
				HandleControllerConnections(input);
			}
		}

		void Game::Update(Renderer &renderer, const Input &input) {
			if (!isEditor) {
				SetExternalForces();
				ProcessPlayerInput(input);
				MovePlayers(input);
			}
		}

		void Game::Render(Renderer &renderer) {
			renderer.BeginFrame();

			const Vec2f tileExt = Vec2f(TileSize * 0.5f);

			if (isEditor) {
				// Draw tiles
				for (u32 y = 0; y < TileCountForHeight; ++y) {
					for (u32 x = 0; x < TileCountForWidth; ++x) {
						const Tile &tile = tiles[y * TileCountForWidth + x];
						if (tile.isSolid) {
							Vec2f tileWorldPos = TileToWorld(x, y);
							renderer.DrawRectangle(tileWorldPos, tileExt, Vec4f::Yellow);
						}
					}
				}
			} else {
				// Draw walls
				for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
					const Wall &wall = walls[wallIndex];
					Vec4f platformColor;
					if (wall.isPlatform)
						platformColor = Vec4f(0.0f, 0.0f, 0.75f);
					else
						platformColor = Vec4f(0.0f, 0.0f, 1.0f);
					renderer.DrawRectangle(wall.position, wall.ext, platformColor);
				}
			}

			// Draw players
			for (u32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
				const Entity &player = players[playerIndex];
				Vec4f playerColor = Vec4f(1.0f, 1.0f, 1.0f);
				renderer.DrawRectangle(player.position, player.ext, playerColor);
			}

			// Draw mouse tile
			if (isEditor) {
				Vec2i mouseTile = WorldToTile(mouseWorldPos);
				Vec2f tileWorldPos = TileToWorld(mouseTile);
				renderer.DrawRectangle(tileWorldPos, tileExt, Vec4f::White, false);
			}

			renderer.EndFrame();
		}
	}
}
#include "game.h"

#include <final_platform_layer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "final_utils.h"

using namespace finalspace::renderer;
using namespace finalspace::utils;

namespace finalspace {
	namespace games {

		struct WallSide {
			f32 plane;
			// NOTE: Relative position in minkowski space
			f32 relX;
			f32 relY;
			// NOTE: Delta movement
			f32 deltaX;
			f32 deltaY;
			// NOTE: Min/Max corners
			f32 minY;
			f32 maxY;
			// NOTE: Surface normal
			Vec2f normal;
		};

		static void *AllocateTexture(const u32 width, const u32 height, void *data) {
			GLuint handle;
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexImage2D(GL_TEXTURE_2D, 0,
				GL_RGBA8,
				width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glBindTexture(GL_TEXTURE_2D, 0);

			assert(sizeof(handle) <= sizeof(void *));
			void *result = ValueToPointer(handle);
			return(result);
		}

		void Game::Release() {
			GLuint textureId = PointerToValue<GLuint>(texture.handle);
			if (textureId) {
				glDeleteTextures(1, &textureId);
				texture = {};
			}
		}

		static Texture LoadTexture(const char *imageFilePath) {
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
						result.handle = AllocateTexture(imageWidth, imageHeight, imageData);
						result.width = imageWidth;
						result.height = imageHeight;
						stbi_image_free(imageData);
					}
				}
				fpl::files::CloseFile2(imageFileHandle);
			}
			return(result);
		}

		void Game::Init() {
			fpl::window::SetWindowTitle("GameDev Challenge Oct 2017");

			constexpr char *imageFilePath = "brickwall.png";
			texture = LoadTexture(imageFilePath);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			gravity = Vec2f(0, -4);

			// Single player for now
			Entity player = Entity();
			player.position = Vec2f();
			player.ext = Vec2f(0.5f, 0.5f);
			player.horizontalSpeed = 20.0f;
			player.horizontalDrag = 13.0f;
			player.canJump = true;
			player.jumpPower = 140.0f;
			players.emplace_back(player);

			// Fixed level for now
			float wallDepth = 0.5f;


			Wall wall;

			// Solid side walls
			wall = Wall();
			wall.position = Vec2f(0, -HalfGameHeight + wallDepth * 0.5f);
			wall.ext = Vec2f(HalfGameWidth, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(0, HalfGameHeight - wallDepth * 0.5f);
			wall.ext = Vec2f(HalfGameWidth, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(-HalfGameWidth + wallDepth * 0.5f, 0);
			wall.ext = Vec2f(wallDepth * 0.5f, HalfGameHeight - (wallDepth));
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(HalfGameWidth - wallDepth * 0.5f, 0);
			wall.ext = Vec2f(wallDepth * 0.5f, HalfGameHeight - (wallDepth));
			walls.emplace_back(wall);

			// Small solid tiles
			wall = Wall();
			wall.position = Vec2f(-HalfGameWidth + 2.0f, -3.25f);
			wall.ext = Vec2f(wallDepth * 0.5f, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(HalfGameWidth - 2.0f, -3.25f);
			wall.ext = Vec2f(wallDepth * 0.5f, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(2.0f, 3.25f);
			wall.ext = Vec2f(wallDepth * 0.5f, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(-2.0f, 3.25f);
			wall.ext = Vec2f(wallDepth * 0.5f, wallDepth * 0.5f);
			walls.emplace_back(wall);

			// Center platforms
			wall = Wall();
			wall.position = Vec2f(0, -3.0f);
			wall.ext = Vec2f(4.0f, wallDepth * 0.5f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			wall = Wall();
			wall.position = Vec2f(0, 1.25f);
			wall.ext = Vec2f(4.0f, wallDepth * 0.5f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			// Side platforms
			wall = Wall();
			wall.ext = Vec2f(1.5f, wallDepth * 0.5f);
			wall.position = Vec2f(-HalfGameWidth + wall.ext.w + wallDepth, -1.0f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			wall = Wall();
			wall.ext = Vec2f(1.5f, wallDepth * 0.5f);
			wall.position = Vec2f(HalfGameWidth - wall.ext.w - wallDepth, -1.0f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			wall = Wall();
			wall.ext = Vec2f(1.0f, wallDepth * 0.5f);
			wall.position = Vec2f(0.0f, -1.0f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			wall = Wall();
			wall.ext = Vec2f(1.0f, wallDepth * 0.5f);
			wall.position = Vec2f(-HalfGameWidth + wall.ext.w + wallDepth, 3.0f);
			wall.isPlatform = true;
			walls.emplace_back(wall);

			wall = Wall();
			wall.ext = Vec2f(1.0f, wallDepth * 0.5f);
			wall.position = Vec2f(HalfGameWidth - wall.ext.w - wallDepth, 3.0f);
			wall.isPlatform = true;
			walls.emplace_back(wall);
		}

		void Game::Render(RenderState &render) {
			glViewport(0, 0, render.windowSize.w, render.windowSize.h);

			// @TODO: Migrate to modern opengl later!

		#if 0
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-HalfGameWidth, HalfGameWidth, -HalfGameHeight, HalfGameHeight, 0.0f, 1.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		#else
			Mat4f proj = Mat4f::CreateOrthoRH(-HalfGameWidth, HalfGameWidth, -HalfGameHeight, HalfGameHeight, 0.0f, 1.0f);
			Mat4f model = Mat4f::Identity;
			Mat4f mv = proj * model;
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(&mv.m[0]);
		#endif

			glClear(GL_COLOR_BUFFER_BIT);

			// Draw walls
			for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
				const Wall &wall = walls[wallIndex];
				if (wall.isPlatform)
					glColor3f(0.0f, 0.0f, 0.75f);
				else
					glColor3f(0.0f, 0.0f, 1.0f);
				glPushMatrix();
				glTranslatef(wall.position.x, wall.position.y, 0.0f);
				glBegin(GL_QUADS);
				glVertex2f(wall.ext.w, wall.ext.h);
				glVertex2f(-wall.ext.w, wall.ext.h);
				glVertex2f(-wall.ext.w, -wall.ext.h);
				glVertex2f(wall.ext.w, -wall.ext.h);
				glEnd();
				glPopMatrix();
			}

			// Draw players
			for (u32 playerIndex = 0; playerIndex < players.size(); ++playerIndex) {
				const Entity &player = players[playerIndex];
				glColor3f(1.0f, 1.0f, 1.0f);
				glPushMatrix();
				glTranslatef(player.position.x, player.position.y, 0.0f);
				glBegin(GL_QUADS);
				glVertex2f(player.ext.w, player.ext.h);
				glVertex2f(-player.ext.w, player.ext.h);
				glVertex2f(-player.ext.w, -player.ext.h);
				glVertex2f(player.ext.w, -player.ext.h);
				glEnd();
				glPopMatrix();
			}

#if 0
			GLuint texHandle = PointerToValue<GLuint>(texture.handle);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texHandle);

			f32 tw = 4.0f;
			f32 th = 4.0f;
			glPushMatrix();
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(tw, th);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(-tw, th);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(-tw, -th);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(tw, -th);
			glEnd();
			glPopMatrix();

			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
#endif
		}

		void Game::Update(const Input &input) {
			const Controller &playerController = input.controllers[input.playerOneControllerIndex];

			Entity &player = players[playerController.playerIndex];
			b32 wasGrounded = player.isGrounded;
			player.isGrounded = false;

			// Set acceleration based on player input
			Vec2f acceleration = {};
			if (playerController.actionLeft.isDown) {
				acceleration.x = -1.0f * player.horizontalSpeed;
			} else if (playerController.actionRight.isDown) {
				acceleration.x = 1.0f * player.horizontalSpeed;
			}

			// Jump
			if (playerController.actionUp.isDown && player.canJump) {
				if (wasGrounded) {
					acceleration.y = 1.0f * player.jumpPower;
				}
			}

			// Gravity
			acceleration += gravity;

			// Horizontal drag
			acceleration += -Dot(Vec2f::Right, player.velocity) * Vec2f::Right * player.horizontalDrag;

			// Movement equation:
			// p' = (a / 2) * dt^2 + v * dt + p
			// v' = a * dt + v
			Vec2f playerDelta = 0.5f * acceleration * (input.deltaTime * input.deltaTime) + player.velocity * input.deltaTime;
			player.velocity = acceleration * input.deltaTime + player.velocity;

			// Do line segment tests for each wall, find the side of the wall which is nearest in time
			// To enable colliding with multiple walls, we iterate it over a few times
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
							{ minCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y, { -1, 0 } },
							{ maxCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y, { 1, 0 } },
							{ minCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x, { 0, -1 } },
							{ maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x, { 0, 1 } },
						};
						u32 wallSideCount = 4;
						if (wall.isPlatform) {
							wallSideCount = 1;
							testSides[0] = { maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x, { 0, 1 } };
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

						// Are we grounded?
						player.isGrounded = Dot(wallNormal, Vec2f::Up) > 0;
					}

				}

			}

		}
	}
}
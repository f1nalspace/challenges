#include "game.h"

#define FPL_ENABLE_WINDOW 1
#include <final_platform_layer.hpp>
#include "final_utils.h"

using namespace finalspace::renderer;
using namespace finalspace::utils;

namespace finalspace {
	namespace games {

		constexpr f32 GameAspect = 16.0f / 9.0f;
		constexpr f32 GameWidth = 20.0f;
		constexpr f32 GameHeight = GameWidth / GameAspect;
		constexpr f32 HalfGameWidth = GameWidth * 0.5f;
		constexpr f32 HalfGameHeight = GameHeight * 0.5f;

		struct WallSide
		{
			f32 x;
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

		void Game::Init() {
			glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

			gravity = { 0, -4 };

			// Single player for now
			Entity player = Entity();
			player.position = Vec2f();
			player.ext = { 0.5f, 0.5f };
			player.horizontalSpeed = 20.0f;
			player.horizontalDrag = 13.0f;
			player.canJump = true;
			player.jumpPower = 160.0f;
			players.emplace_back(player);

			// Fixed level for now
			float wallDepth = 0.5f;

			Wall wall = Wall();
			wall.position.y = -HalfGameHeight + wallDepth * 0.5f;
			wall.ext = { HalfGameWidth, wallDepth * 0.5f };
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.y = HalfGameHeight - wallDepth * 0.5f;
			wall.ext = { HalfGameWidth, wallDepth * 0.5f };
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.x = -HalfGameWidth + wallDepth * 0.5f;
			wall.ext = { wallDepth * 0.5f, HalfGameHeight - (wallDepth) };
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.x = HalfGameWidth - wallDepth * 0.5f;
			wall.ext = { wallDepth * 0.5f, HalfGameHeight - (wallDepth) };
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.y = -2.5f;
			wall.ext = { 4.0f, wallDepth * 0.5f };
			wall.isPlatform = true;
			walls.emplace_back(wall);
		}

		void Game::Render(RenderState &render) {
			glViewport(0, 0, render.windowSize.w, render.windowSize.h);

			// @TODO: Move later to modern opengl!

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-HalfGameWidth, HalfGameWidth, -HalfGameHeight, HalfGameHeight, 0.0f, 1.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glClear(GL_COLOR_BUFFER_BIT);

			// Draw walls
			for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
				const Wall &wall = walls[wallIndex];
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
				glVertex2f(0.5f, 0.5f);
				glVertex2f(-0.5f, 0.5f);
				glVertex2f(-0.5f, -0.5f);
				glVertex2f(0.5f, -0.5f);
				glEnd();
				glPopMatrix();
			}
		}

		void Game::Update(const Input &input) {
			const Controller &playerController = input.controllers[input.playerOneControllerIndex];

			const Vec2f up = { 0, 1 };
			const Vec2f right = { 1, 0 };

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
			acceleration += -Dot(right, player.velocity) * right * player.horizontalDrag;

			// Movement equation:
			// p' = (a / 2) * dt^2 + v * dt + p
			// v' = a * dt + v
			Vec2f playerDelta = 0.5f * acceleration * (input.deltaTime * input.deltaTime) + player.velocity * input.deltaTime;
			player.velocity = acceleration * input.deltaTime + player.velocity;

			// Dont assume we are grounded
			player.isGrounded = false;

			// Do line segment tests for each wall, find the side of the wall which is the nearest in time
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
							constexpr f32 epsilon = 0.001f;
							if (testSide->deltaX != 0.0f)
							{
								f32 f = (testSide->x - testSide->relX) / testSide->deltaX;
								f32 y = testSide->relY + f*testSide->deltaY;
								if ((f >= 0.0f) && (hitTime > f))
								{
									if ((y >= testSide->minY) && (y <= testSide->maxY))
									{
										hitTime = Maximum(0.0f, f - epsilon);
										testSideNormal = testSide->normal;
										wasHit = true;
									}
								}
							}
						}

						if (wasHit)
						{
							f32 d = Dot(playerDelta, up);
							if ((!wall.isPlatform) || (wall.isPlatform && (d <= 0))) {
								tmin = hitTime;
								wallNormalMin = testSideNormal;
								hitWallMin = &wall;
							}
						}
					}

					Vec2f wallNormal = Vec2f();
					Wall *hitWall = nullptr;
					f32 stopTime;
					if (tmin < tmax)
					{
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
						player.isGrounded = (wallNormal.x == 0) && (wallNormal.y == 1);
					}

				}

			}

		}
	}
}
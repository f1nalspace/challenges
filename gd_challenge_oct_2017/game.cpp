#include "game.h"

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
			f32 relX;
			f32 relY;
			f32 deltaX;
			f32 deltaY;
			f32 minY;
			f32 maxY;
			Vec2f normal;
		};

		void Game::Init() {
			glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

			gravity = Vec2f(0, -10);

			player = Player();
			player.ext = Vec2f(0.5f, 0.5f);

			float wallDepth = 0.5f;

			Wall wall = Wall();
			wall.position.y = -HalfGameHeight + wallDepth * 0.5f;
			wall.ext = Vec2f(HalfGameWidth, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.y = HalfGameHeight - wallDepth * 0.5f;
			wall.ext = Vec2f(HalfGameWidth, wallDepth * 0.5f);
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.x = -HalfGameWidth + wallDepth * 0.5f;
			wall.ext = Vec2f(wallDepth * 0.5f, HalfGameHeight - (wallDepth));
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.x = HalfGameWidth - wallDepth * 0.5f;
			wall.ext = Vec2f(wallDepth * 0.5f, HalfGameHeight - (wallDepth));
			walls.emplace_back(wall);

			wall = Wall();
			wall.position.y = -2.0f;
			wall.ext = Vec2f(4.0f, wallDepth * 0.5f);
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

			// Draw player
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
		}

		void Game::Update(const Input &input) {
			const Controller &playerController = input.controller[input.playerControllerIndex];
			Vec2f playerAcceleration = Vec2f();
			f32 speed = 20.0f;
			if (playerController.moveLeft.isDown) {
				playerAcceleration.x = -1.0f;
			} else if (playerController.moveRight.isDown) {
				playerAcceleration.x = 1.0f;
			}
			if (playerController.moveUp.isDown) {
				playerAcceleration.y = 1.0f;
			} else if (playerController.moveDown.isDown) {
				playerAcceleration.y = -1.0f;
			}
			if (playerAcceleration.x != 0 || playerAcceleration.y != 0) {
				playerAcceleration = Normalize(playerAcceleration);
			}
			playerAcceleration *= speed;

			// Add gravity
			playerAcceleration += gravity;

			// Add drag
			playerAcceleration += -player.velocity * 2.0f;

			// Movement equation:
			// p' = (a / 2) * dt^2 + v * dt + p
			// v' = a * dt + v
			Vec2f playerDelta = 0.5f * playerAcceleration * (input.deltaTime * input.deltaTime) + player.velocity * input.deltaTime;
			player.velocity = playerAcceleration * input.deltaTime + player.velocity;

			for (u32 iteration = 0; iteration < 4; ++iteration) {
				f32 tmin = 1.0f;
				f32 tmax = 1.0f;

				f32 playerDeltaLen = GetLength(playerDelta);
				if (playerDeltaLen > 0.0f) {

					Vec2f wallNormalMin = Vec2f();
					Wall *hitWallMin = nullptr;

					Vec2f targetPosition = player.position + playerDelta;

					for (u32 wallIndex = 0; wallIndex < walls.size(); ++wallIndex) {
						Wall &thisWall = walls[wallIndex];

						Vec2f minkowskiExt = Vec2f(player.ext.x + thisWall.ext.x, player.ext.y + thisWall.ext.y);
						Vec2f minCorner = -minkowskiExt;
						Vec2f maxCorner = minkowskiExt;

						Vec2f rel = player.position - thisWall.position;

						WallSide testSides[] =
						{
							{ minCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y, Vec2f(-1, 0) },
							{ maxCorner.x, rel.x, rel.y, playerDelta.x, playerDelta.y, minCorner.y, maxCorner.y, Vec2f(1, 0) },
							{ minCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x, Vec2f(0, -1) },
							{ maxCorner.y, rel.y, rel.x, playerDelta.y, playerDelta.x, minCorner.x, maxCorner.x, Vec2f(0, 1) },
						};

						// @TODO: It works  but i would prefered a generic line segment intersection test here
						Vec2f testSideNormal = Vec2f();
						f32 hitTime = tmin;
						bool wasHit = false;
						for (u64 testSideIndex = 0; testSideIndex < ArrayCount(testSides); ++testSideIndex) {
							WallSide *testSide = testSides + testSideIndex;
							f32 tEpsilon = 0.001f;
							if (testSide->deltaX != 0.0f)
							{
								f32 f = (testSide->x - testSide->relX) / testSide->deltaX;
								f32 y = testSide->relY + f*testSide->deltaY;
								if ((f >= 0.0f) && (hitTime > f))
								{
									if ((y >= testSide->minY) && (y <= testSide->maxY))
									{
										hitTime = Maximum(0.0f, f - tEpsilon);
										testSideNormal = testSide->normal;
										wasHit = true;
									}
								}
							}
						}

						if (wasHit)
						{
							tmin = hitTime;
							wallNormalMin = testSideNormal;
							hitWallMin = &thisWall;
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
					}

				}

			}

		}
	}
}
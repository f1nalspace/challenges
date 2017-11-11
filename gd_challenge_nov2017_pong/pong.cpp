#include "pong.h"

#include <final_platform_layer.hpp>
#include "final_collisions.h"

using namespace fs::collisions;

namespace fs {
	namespace games {
		constexpr f32 PlaneAreaScale = 0.9f;
		constexpr f32 NormalArrowSize = 0.25f;
		constexpr f32 EpsilonTime = 0.001f;

		Pong::Pong() {
			title = "Pongy";
		}
		Pong::~Pong() {

		}

		void Pong::ResetBall() {
			f32 angle = RandomUnilateral(entropy) * TAU32;
			Vec2f direction = Vec2f(Cosine(angle), Sine(angle));
			ball.moveable.speed = ball.moveable.initialSpeed;
			ball.moveable.velocity = direction * ball.moveable.speed;
			ball.moveable.position = ball.moveable.initialPosition;
			ball.moveable.acceleration = Vec2f();
		}

		void Pong::Init() {
			glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			planes.emplace_back(Plane(PlaneType::LeftSide, Vec2f(1, 0), -GAME_HALF_WIDTH * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));
			planes.emplace_back(Plane(PlaneType::RightSide, Vec2f(-1, 0), -GAME_HALF_WIDTH * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));
			planes.emplace_back(Plane(PlaneType::None, Vec2f(0, 1), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_WIDTH * PlaneAreaScale));
			planes.emplace_back(Plane(PlaneType::None, Vec2f(0, -1), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_WIDTH * PlaneAreaScale));

			entropy = RandomSeed(1337);

			f32 dt = 1.0f / 60.0f;
			ball = Ball();
			ball.moveable.initialPosition = Vec2f(0, 0);
			ball.moveable.initialSpeed = 2.0f;
			ResetBall();
		}

		void Pong::Release() {
			controlledPlayers.clear();
			paddles.clear();
			planes.clear();
		}

		void Pong::HandleInput(const Input &input) {
			HandleControllerConnections(input);
		}

		void Pong::SetExternalForces(const f32 dt) {
			ball.moveable.acceleration = Vec2f();
			u32 paddleIndex = 0;
			for (Paddle &paddle : paddles) {
				paddle.moveable.acceleration = Vec2f();
				paddle.moveable.acceleration += -Dot(Vec2f::Up, paddle.moveable.velocity) * Vec2f::Up * paddle.verticalDrag;
				++paddleIndex;
			}
		}

		u32 Pong::CreatePaddle(const u32 planeIndex, const Vec4f &color) {
			Vec2f n = planes[planeIndex].normal;
			Paddle paddle;
			paddle = Paddle();
			paddle.verticalDrag = 40.0;
			paddle.color = color;
			paddle.moveable.speed = 20.0f;
			paddle.moveable.initialPosition = (n * planes[planeIndex].distance) + Hadamard(n, paddle.ext) * 2.0f;
			paddle.moveable.position = paddle.moveable.initialPosition;
			paddles.emplace_back(paddle);
			u32 result = (u32)(paddles.size() - 1);
			return(result);
		}

		s32 Pong::FindControlledPlayerIndex(const u32 controllerIndex) {
			s32 result = -1;
			for (u32 controlledPlayerIndex = 0; controlledPlayerIndex < controlledPlayers.size(); ++controlledPlayerIndex) {
				if (controlledPlayers[controlledPlayerIndex].controllerIndex == controllerIndex) {
					result = controlledPlayerIndex;
					break;
				}
			}
			return(result);
		}

		void Pong::HandleControllerConnections(const Input &input) {
			for (u32 controllerIndex = 0; controllerIndex < utils::ArrayCount(input.controllers); ++controllerIndex) {
				const Controller &testController = input.controllers[controllerIndex];
				if (testController.isConnected) {

					// @NOTE: Connected controller
					s32 foundControlledPlayerIndex = FindControlledPlayerIndex(controllerIndex);
					if (foundControlledPlayerIndex == -1) {
						u32 playerIndex;
						if (!isStarted) {
							assert(paddles.size() == 0);
							playerIndex = CreatePaddle(0, Vec4f::Blue); // Player
							CreatePaddle(1, Vec4f::Red); // AI
							isStarted = true;
						} else {
							playerIndex = 0;
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
						paddles.erase(paddles.begin() + playerIndex);
						controlledPlayers.erase(controlledPlayers.begin() + foundControlledPlayerIndex);
					}
				}
			}
		}

		void Pong::HandlePlayerInput(const Input &input) {
			for (u32 controlledPlayerIndex = 0; controlledPlayerIndex < controlledPlayers.size(); ++controlledPlayerIndex) {
				u32 playerIndex = controlledPlayers[controlledPlayerIndex].playerIndex;
				u32 controllerIndex = controlledPlayers[controlledPlayerIndex].controllerIndex;

				// @BUG: On app shutdown this crashes due to the assert below.
				// Players got cleared but connected controllers arent,
				// because the controller is not disconnected when shutting down.
				// This condition ensures that it wont crash, but its not correct!
				if (playerIndex >= (paddles.size())) continue;

				assert(playerIndex < paddles.size());
				assert(controllerIndex < utils::ArrayCount(input.controllers));

				Paddle &paddle = paddles[playerIndex];
				const Controller &playerController = input.controllers[controllerIndex];

				if (playerController.moveDown.isDown) {
					paddles[0].moveable.acceleration.y = -1.0f * paddles[0].moveable.speed;
				} else if (playerController.moveUp.isDown) {
					paddles[0].moveable.acceleration.y = 1.0f * paddles[0].moveable.speed;
				}
			}
		}

		void Pong::HandleBallCollisions(Ball &ball, Entity *enemy) {
			if (enemy->entityType == EntityType::Plane) {
				Plane *plane = (Plane *)enemy;
				if ((plane->planeType == PlaneType::LeftSide) ||
					(plane->planeType == PlaneType::RightSide)) {
					ResetBall();
				}
			}
		}

		void Pong::MoveEntities(const f32 dt) {
			// Integrate ball
			ball.moveable.delta = 0.5f * ball.moveable.acceleration * (dt * dt) + ball.moveable.velocity * dt;
			ball.moveable.velocity = ball.moveable.acceleration * dt + ball.moveable.velocity;

			// Integrate paddles
			for (Paddle &paddle : paddles) {
				paddle.moveable.delta = 0.5f * paddle.moveable.acceleration * (dt * dt) + paddle.moveable.velocity * dt;
				paddle.moveable.velocity = paddle.moveable.acceleration * dt + paddle.moveable.velocity;
			}

			// Ball collision
			{
				f32 tMax = 1.0f;
				for (u32 iteration = 0; iteration < 4; ++iteration) {
					f32 tMin = tMax;
					Vec2f hitNormal = Vec2f();
					bool wasHit = false;
					Entity *hitEntity = nullptr;

					Vec2f targetPosition = ball.moveable.position + ball.moveable.delta;

					// Planes
					for (Plane &plane : planes) {
						Vec2f planeCenter = plane.normal * plane.distance;
						Vec2f relativePosition = ball.moveable.position - planeCenter;
						f32 distanceToPlane = Dot(plane.normal, relativePosition) - ball.radius;
						f32 projMovement = -Dot(plane.normal, ball.moveable.delta);
						if (Absolute(projMovement) > 0) {
							f32 f = distanceToPlane / projMovement;
							if ((f >= 0.0f) && (tMin > f)) {
								tMin = Maximum(0.0f, f - EpsilonTime);
								hitNormal = plane.normal;
								wasHit = true;
								hitEntity = &plane.entity;
							}
						}
					}

					// Paddles
					for (Paddle &paddle : paddles) {
						Vec2f relativePosition = ball.moveable.position - paddle.moveable.position;

						f32 minkowskiRight = (paddle.ext.w + ball.radius);
						f32 minkowskiLeft = -(paddle.ext.w + ball.radius);
						f32 minkowskiTop = (paddle.ext.h + ball.radius);
						f32 minkowskiBottom = -(paddle.ext.h + ball.radius);

						constexpr u32 segmentCount = 4;
						DeltaPlane2D paddleSegments[segmentCount] = {
							{ minkowskiRight, relativePosition.x, relativePosition.y, ball.moveable.delta.x, ball.moveable.delta.y, minkowskiBottom, minkowskiTop, Vec2f(1, 0) },
							{ minkowskiLeft, relativePosition.x, relativePosition.y, ball.moveable.delta.x, ball.moveable.delta.y, minkowskiBottom, minkowskiTop, Vec2f(-1, 0) },
							{ minkowskiTop, relativePosition.y, relativePosition.x, ball.moveable.delta.y, ball.moveable.delta.x, minkowskiLeft, minkowskiRight, Vec2f(0, 1) },
							{ minkowskiBottom, relativePosition.y, relativePosition.x, ball.moveable.delta.y, ball.moveable.delta.x, minkowskiLeft, minkowskiRight, Vec2f(0, -1) },
						};

						IntersectionResult intersection = IntersectLines(tMin, EpsilonTime, segmentCount, paddleSegments);
						if (intersection.wasHit) {
							f32 f = intersection.tMin;
							if ((f >= 0.0f) && (tMin > f)) {
								tMin = Maximum(0.0f, f - EpsilonTime);
								hitNormal = intersection.normal;
								wasHit = true;
								hitEntity = &paddle.entity;
							}
						}
					}

					ball.moveable.position += tMin * ball.moveable.delta;

					if (wasHit) {
						f32 restitution = 1.0f;
						ball.moveable.delta = targetPosition - ball.moveable.position;
						ball.moveable.delta += -(1 + restitution) * Dot(ball.moveable.delta, hitNormal) * hitNormal;
						ball.moveable.velocity += -(1 + restitution) * Dot(ball.moveable.velocity, hitNormal) * hitNormal;
						tMax = tMin;
						assert(hitEntity != nullptr);
						HandleBallCollisions(ball, hitEntity);
					}
				}

				// Paddle movements and collisions
				for (Paddle &paddle : paddles) {
					f32 tMax = 1.0f;
					for (u32 iteration = 0; iteration < 4; ++iteration) {
						f32 tMin = tMax;
						Vec2f hitNormal = Vec2f();
						bool wasHit = false;
						Entity *hitEntity = nullptr;

						Vec2f targetPosition = paddle.moveable.position + paddle.moveable.delta;

						// Planes
						for (Plane &plane : planes) {
							Vec2f planeCenter = plane.normal * plane.distance;
							Vec2f relativePosition = paddle.moveable.position - planeCenter;
							f32 distanceToPlane = Dot(plane.normal, relativePosition) - Absolute(Dot(paddle.ext, plane.normal));
							f32 projMovement = -Dot(plane.normal, paddle.moveable.delta);
							if (Absolute(projMovement) > 0) {
								f32 f = distanceToPlane / projMovement;
								if ((f >= 0.0f) && (tMin > f)) {
									tMin = Maximum(0.0f, f - EpsilonTime);
									hitNormal = plane.normal;
									wasHit = true;
									hitEntity = &plane.entity;
								}
							}
						}

						paddle.moveable.position += tMin * paddle.moveable.delta;

						if (wasHit) {
							f32 restitution = 0.0f;
							paddle.moveable.delta = targetPosition - paddle.moveable.position;
							paddle.moveable.delta += -(1 + restitution) * Dot(paddle.moveable.delta, hitNormal) * hitNormal;
							paddle.moveable.velocity += -(1 + restitution) * Dot(paddle.moveable.velocity, hitNormal) * hitNormal;
							tMax = tMin;
						}
					}
				}
			}
		}

		void Pong::UpdateAI(const f32 dt) {
			if (paddles.size() == 2) {
				Paddle &paddle = paddles[1];
				Vec2f target;
				f32 ballHorizontalMovement = Dot(Vec2f::Right, ball.moveable.position);
				if (ballHorizontalMovement > 0) {
					target = ball.moveable.position;
				} else {
					target = paddle.moveable.initialPosition;
				}
				Vec2f relPos = paddle.moveable.position - target;
				f32 projPos = Dot(Vec2f::Up, relPos);
				f32 a = 2.0f * projPos * (1.0f / dt);
				Vec2f accel = Vec2f(Vec2f::Up) * -a;
				paddle.moveable.acceleration += accel;
			}
		}

		void Pong::Update(const Input &input) {
			SetExternalForces(input.deltaTime);
			HandlePlayerInput(input);
			UpdateAI(input.deltaTime);
			MoveEntities(input.deltaTime);
		}

		void Pong::Render(const Input &input) {
			const f32 dt = input.deltaTime;

			renderer->Update(GAME_WIDTH * 0.5f, GAME_HEIGHT * 0.5f, GAME_ASPECT);
			renderer->BeginFrame();

			renderer->DrawRectangle(Vec2f(0, 0), Vec2f(GAME_WIDTH, GAME_HEIGHT) * 0.5f, Vec4f::White, false, 2.0f);

			for (const Paddle &paddle : paddles) {
				renderer->DrawRectangle(paddle.moveable.position, paddle.ext, paddle.color, true, 2.0f);
			}

			for (const Plane &plane : planes) {
				Vec2f tangent = Cross(1.0f, plane.normal);
				Vec2f center = plane.normal * plane.distance;
				Vec2f a = center + tangent * plane.visualLength * 0.5f;
				Vec2f b = center + tangent * -plane.visualLength * 0.5f;
				renderer->DrawLine(a, b, Vec4f::Blue, 2.0f);
				renderer->DrawLine(center, center + plane.normal * NormalArrowSize, Vec4f::Red, 2.0f);
			}

			renderer->DrawCircle(ball.moveable.position, ball.radius, ball.color, true, 16, 2.0f);

			renderer->EndFrame();
		}
	}
}
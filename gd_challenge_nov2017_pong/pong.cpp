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
		void Pong::Init() {
			glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			planes.emplace_back(Plane(Vec2f(1, 0), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));
			planes.emplace_back(Plane(Vec2f(0, 1), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));
			planes.emplace_back(Plane(Vec2f(-1, 0), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));
			planes.emplace_back(Plane(Vec2f(0, -1), -GAME_HALF_HEIGHT * PlaneAreaScale, GAME_HEIGHT * PlaneAreaScale));

			f32 dt = 1.0f / 60.0f;
			ball = Ball();
			ball.acceleration = Normalize(Vec2f(1, 1)) * 500.0f;
			ball.position = Vec2f(0, 0);

			paddle = Paddle();
			paddle.position = (planes[0].normal * planes[0].distance) + Hadamard(Vec2f(1, 0), paddle.ext) * 2.0f;
		}
		void Pong::Release() {

		}
		void Pong::HandleInput(const Input &input) {

		}
		void Pong::Update(const Input &input) {
		#if 0
			const f32 dt = input.deltaTime;

			ball.delta = 0.5f * ball.acceleration * (dt * dt) + ball.velocity * dt;
			ball.velocity = ball.acceleration * dt + ball.velocity;
			ball.acceleration = Vec2f();

			f32 tMax = 1.0f;
			for (u32 iteration = 0; iteration < 4; ++iteration) {
				f32 tMin = tMax;
				Vec2f hitNormal = Vec2f();
				bool wasHit = false;

				Vec2f targetPosition = ball.position + ball.delta;

				for (const Plane &plane : planes) {
					Vec2f planeCenter = plane.normal * plane.distance;
					Vec2f relativePosition = ball.position - planeCenter;
					f32 distanceToPlane = Dot(plane.normal, relativePosition) - ball.radius;
					f32 projMovement = -Dot(plane.normal, ball.delta);
					if (Absolute(projMovement) > 0) {
						f32 f = distanceToPlane / projMovement;
						if ((f >= 0.0f) && (tMin > f)) {
							tMin = Maximum(0.0f, f - EpsilonTime);
							hitNormal = plane.normal;
							wasHit = true;
						}
					}
				}

				ball.position += tMin * ball.delta;

				if (wasHit) {
					f32 restitution = 1.0f;
					ball.delta = targetPosition - ball.position;
					ball.delta += -(1 + restitution) * Dot(ball.delta, hitNormal) * hitNormal;
					ball.velocity += -(1 + restitution) * Dot(ball.velocity, hitNormal) * hitNormal;
					tMax = tMin;
				}
			}
		#endif
		}
		void Pong::Render(const Input &input) {
			const f32 dt = input.deltaTime;

			renderer->Update(GAME_WIDTH * 0.5f, GAME_HEIGHT * 0.5f, GAME_ASPECT);
			renderer->BeginFrame();

			renderer->DrawRectangle(Vec2f(0, 0), Vec2f(GAME_WIDTH, GAME_HEIGHT) * 0.5f, Vec4f::White, false, 2.0f);

			renderer->DrawCircle(ball.position, ball.radius, Vec4f::Blue, false, 16, 2.0f);

			renderer->DrawRectangle(paddle.position, paddle.ext, Vec4f::Blue, false, 2.0f);

			for (const Plane &plane : planes) {
				Vec2f tangent = Cross(1.0f, plane.normal);
				Vec2f center = plane.normal * plane.distance;
				Vec2f a = center + tangent * plane.visualLength * 0.5f;
				Vec2f b = center + tangent * -plane.visualLength * 0.5f;
				renderer->DrawLine(a, b, Vec4f::Blue, 2.0f);
				renderer->DrawLine(center, center + plane.normal * NormalArrowSize, Vec4f::Red, 2.0f);
			}

			Vec2f targetBallPos = ball.position + Vec2f(-1, 0) * (GAME_HALF_HEIGHT * PlaneAreaScale - ball.radius * 0.75f);
			ball.acceleration = 2.0f * (targetBallPos - ball.position) * (1.0f / (dt * dt));
			renderer->DrawCircle(targetBallPos, ball.radius, Vec4f::Red, false, 16, 2.0f);

			Vec2f targetPaddlePos = paddle.position + Vec2f(1, 0) * 2.0f;
			paddle.acceleration = 2.0f * (targetPaddlePos - paddle.position) * (1.0f / (dt * dt));
			renderer->DrawRectangle(targetPaddlePos, paddle.ext, Vec4f::Red, false, 2.0f);

			{
				Vec2f ballDelta = 0.5f * ball.acceleration * (dt * dt) + ball.velocity * dt;
				Vec2f paddleDelta = 0.5f * paddle.acceleration * (dt * dt) + paddle.velocity * dt;
				Vec2f relativeMovement = ballDelta - paddleDelta;
				Vec2f relativePosition = ball.position - paddle.position;

				f32 minkowskiRight = (paddle.ext.w + ball.radius);
				f32 minkowskiLeft = -(paddle.ext.w + ball.radius);
				f32 minkowskiTop = (paddle.ext.h + ball.radius);
				f32 minkowskiBottom = -(paddle.ext.h + ball.radius);

				DeltaPlane2D deltaPlane;
				deltaPlane.plane = minkowskiRight;
				deltaPlane.minY = minkowskiBottom;
				deltaPlane.maxY = minkowskiTop;
				deltaPlane.deltaX = relativeMovement.x;
				deltaPlane.deltaY = relativeMovement.y;
				deltaPlane.relX = relativePosition.x;
				deltaPlane.relY = relativePosition.y;
				f32 tMin = 1.0f;
				IntersectionResult intersection = IntersectLines(tMin, EpsilonTime, 1, &deltaPlane);
				if (intersection.wasHit) {
					tMin = intersection.tMin;
				}
				Vec2f p = relativeMovement * tMin;
				renderer->DrawCircle(p, ball.radius, Vec4f::Yellow, false, 16, 2.0f);
			}

			renderer->EndFrame();
		}
	}
}
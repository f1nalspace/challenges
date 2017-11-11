#pragma once

#include <vector>

#include "final_game.h"
#include "final_maths.h"
#include "final_randoms.h"

using namespace fs::maths;
using namespace fs::randoms;

namespace fs {
	namespace games {
		enum class EntityType {
			None,
			Ball,
			Paddle,
			Plane,
		};

		struct Entity {
			EntityType entityType;
			Entity(const EntityType entityType) : 
				entityType(entityType) {
			}
			Entity(const Entity &entity) :
				entityType(entity.entityType) {
			}
		};

		struct Moveable {
			Vec2f initialPosition = Vec2f();
			Vec2f position = Vec2f();
			Vec2f velocity = Vec2f();
			Vec2f acceleration = Vec2f();
			Vec2f delta = Vec2f();
			f32 speed = 0.0f;
			f32 initialSpeed = 0.0f;
		};

		struct Ball {
			Entity entity = Entity(EntityType::Ball);
			Moveable moveable = Moveable();
			f32 radius = 0.2f;
			Vec4f color = Vec4f::Yellow;
		};

		struct Paddle {
			Entity entity = Entity(EntityType::Paddle);
			Moveable moveable = Moveable();
			Vec2f ext = Vec2f(0.25f, 0.9f);
			f32 verticalDrag = 0.0f;
			Vec4f color = Vec4f::White;
		};

		enum class PlaneType {
			None,
			LeftSide,
			RightSide,
		};

		struct Plane {
			Entity entity = Entity(EntityType::Plane);
			PlaneType planeType = PlaneType::None;
			Vec2f normal = Vec2f();
			f32 distance = 0.0f;
			f32 visualLength = 0.0f;
			Plane() = default;
			Plane(const Plane &other) {
				this->entity = other.entity;
				this->planeType = other.planeType;
				this->normal = other.normal;
				this->distance = other.distance;
				this->visualLength = other.visualLength;
			}
			Plane(const PlaneType planeType, const Vec2f &normal, const f32 distance, const f32 visualLength) {
				this->entity = Entity(EntityType::Plane);
				this->planeType = planeType;
				this->normal = normal;
				this->distance = distance;
				this->visualLength = visualLength; 
			}
		};

		struct ControlledPlayer {
			u32 playerIndex = 0;
			u32 controllerIndex = 0;
		};

		struct Pong : BaseGame {
		private:
			u32 CreatePaddle(const u32 planeIndex, const Vec4f &color);
			void SetExternalForces(const f32 dt);
			void MoveEntities(const f32 dt);
			void HandlePlayerInput(const Input &input);
			void UpdateAI(const f32 dt);
			s32 FindControlledPlayerIndex(const u32 controllerIndex);
			void HandleControllerConnections(const Input &input);
			void ResetBall();
			void HandleBallCollisions(Ball &ball, Entity *enemy);
		public:
			static constexpr f32 GAME_ASPECT = 16.0f / 9.0f;
			static constexpr f32 GAME_WIDTH = 20.0f;
			static constexpr f32 GAME_HEIGHT = GAME_WIDTH / GAME_ASPECT;
			static constexpr f32 GAME_HALF_WIDTH = GAME_WIDTH * 0.5f;
			static constexpr f32 GAME_HALF_HEIGHT = GAME_HEIGHT * 0.5f;

			Ball ball;
			std::vector<Paddle> paddles;
			std::vector<Plane> planes;
			std::vector<ControlledPlayer> controlledPlayers = std::vector<ControlledPlayer>();
			bool isStarted = false;
			RandomSeries entropy;

			Pong();
			~Pong() override;
			void Init() override;
			void Release() override;
			void HandleInput(const Input &input) override;
			void Update(const Input &input) override;
			void Render(const Input &input) override;
		};
	};
};
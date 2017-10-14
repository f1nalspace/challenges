#pragma once

#include <vector>

#include "final_types.h"
#include "final_maths.h"
#include "final_input.h"
#include "final_render.h"

using namespace finalspace::inputs;
using namespace finalspace::maths;
using namespace finalspace::renderer;

namespace finalspace {

	namespace games {

		enum class EntityType {
			Player,
		};

		struct Entity {
			Vec2f position;
			Vec2f velocity;
			Vec2f acceleration;
			Vec2f ext;
			EntityType type;
			b32 isGrounded;
			f32 horizontalSpeed;
			f32 horizontalDrag;
			b32 canJump;
			u32 jumpCount;
			f32 jumpPower;
		};

		struct Wall {
			Vec2f position;
			Vec2f ext;
			b32 isPlatform;
		};

		struct ControlledPlayer {
			u32 playerIndex;
			u32 controllerIndex;
		};

		struct Tile {
			b32 isSolid;
		};

		struct Game {
			static constexpr f32 TileSize = 0.5f;
			static constexpr u32 TileCountForWidth = 40;
			static constexpr u32 TileCountForHeight = 22;
			static constexpr f32 GameAspect = TileCountForWidth / (f32)TileCountForHeight;
			static constexpr f32 GameWidth = TileCountForWidth * TileSize;
			static constexpr f32 GameHeight = GameWidth / GameAspect;
			static constexpr f32 HalfGameWidth = GameWidth * 0.5f;
			static constexpr f32 HalfGameHeight = GameHeight * 0.5f;

			inline Vec2f TileToWorld(const s32 tileX, const s32 tileY) {
				const Vec2f tileMapExt = Vec2f((f32)TileCountForWidth * TileSize, (f32)TileCountForHeight * TileSize) * 0.5f;
				Vec2f result = -tileMapExt +
					Vec2f((f32)tileX, (f32)tileY) * TileSize +
					TileSize * 0.5f;
				return(result);
			}
			inline Vec2f TileToWorld(const Vec2i &tilePos) {
				Vec2f result = TileToWorld(tilePos.x, tilePos.y);
				return(result);
			}

			inline Vec2i WorldToTile(const f32 worldX, const f32 worldY) {
				const Vec2f tileMapExt = Vec2f((f32)TileCountForWidth * TileSize, (f32)TileCountForHeight * TileSize) * 0.5f;
				s32 tileX = (s32)((worldX + tileMapExt.w) / TileSize);
				s32 tileY = (s32)((worldY + tileMapExt.h) / TileSize);
				Vec2i result = Vec2i(tileX, tileY);
				return(result);
			}
			inline Vec2i WorldToTile(const Vec2f &worldPos) {
				Vec2i result = WorldToTile(worldPos.x, worldPos.y);
				return(result);
			}

			Vec2f gravity;
			b32 isSinglePlayer;
			std::vector<Entity> players;
			std::vector<Wall> walls;
			std::vector<ControlledPlayer> controlledPlayers;

			Tile tiles[TileCountForWidth * TileCountForHeight];

			b32 isEditor;
			Vec2f mouseWorldPos;

			// @Temporary: Remove this later when we have a proper asset system
			Texture texture;

			u32 CreatePlayer(const u32 controllerIndex);

			s32 FindControlledPlayerIndex(const u32 controllerIndex);

			void Init(Renderer &renderer);
			void Release(Renderer &renderer);
			void HandleControllerConnections(const finalspace::inputs::Input & input);
			void HandlePlayerInput(const finalspace::inputs::Input & input);
			void MovePlayers(const finalspace::inputs::Input & input);
			void SetExternalForces();
			void EditorUpdate(const finalspace::inputs::Input & input);
			void Update(Renderer &renderer, const Input &input);
			void Render(Renderer &renderer);
		};

	};

};
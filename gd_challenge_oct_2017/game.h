#pragma once

#include <vector>
#include <memory>
#include <string>

// @TODO: Remove this!
#include <final_platform_layer.hpp>

#include "final_types.h"
#include "final_maths.h"
#include "final_input.h"
#include "final_renderer.h"

namespace finalspace {

	inline namespace games {

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
			Vec2f position = {};
			Vec2f ext = {};
			b32 isPlatform = false;
		};

		struct ControlledPlayer {
			u32 playerIndex = 0;
			u32 controllerIndex = 0;
		};

		struct Tile {
			b32 isSolid = false;
		};

		class BaseGame {
		protected:
			Renderer &renderer;
			bool exitRequested;
		public:
			BaseGame(Renderer &renderer) : 
				renderer(renderer),
				exitRequested(false) {
			}
			virtual void Init() = 0;
			virtual void Release() = 0;
			virtual void HandleInput(const Input &input) = 0;
			virtual void Update(const Input &input) = 0;
			virtual void Render() = 0;
			virtual ~BaseGame() {
			}
			inline bool IsExitRequested() const {
				return exitRequested;
			}
		};

		struct Game : BaseGame {
		protected:
			static constexpr f32 TileSize = 0.5f;
			static constexpr u32 TileCountForWidth = 40;
			static constexpr u32 TileCountForHeight = 22;
			static constexpr f32 GameAspect = TileCountForWidth / (f32)TileCountForHeight;
			static constexpr f32 GameWidth = TileCountForWidth * TileSize;
			static constexpr f32 GameHeight = GameWidth / GameAspect;
			static constexpr f32 HalfGameWidth = GameWidth * 0.5f;
			static constexpr f32 HalfGameHeight = GameHeight * 0.5f;

			inline Vec2f TileToWorld(const s32 tileX, const s32 tileY) const {
				const Vec2f tileMapExt = Vec2f((f32)TileCountForWidth * TileSize, (f32)TileCountForHeight * TileSize) * 0.5f;
				Vec2f result = -tileMapExt +
					Vec2f((f32)tileX, (f32)tileY) * TileSize +
					TileSize * 0.5f;
				return(result);
			}
			inline Vec2f TileToWorld(const Vec2i &tilePos) const {
				Vec2f result = TileToWorld(tilePos.x, tilePos.y);
				return(result);
			}

			inline Vec2i WorldToTile(const f32 worldX, const f32 worldY) const {
				const Vec2f tileMapExt = Vec2f((f32)TileCountForWidth * TileSize, (f32)TileCountForHeight * TileSize) * 0.5f;
				s32 tileX = (s32)((worldX + tileMapExt.w) / TileSize);
				s32 tileY = (s32)((worldY + tileMapExt.h) / TileSize);
				Vec2i result = Vec2i(tileX, tileY);
				return(result);
			}

			inline Vec2i WorldToTile(const Vec2f &worldPos) const {
				Vec2i result = WorldToTile(worldPos.x, worldPos.y);
				return(result);
			}

			inline void SetTile(const u32 x, const u32 y, const bool value) {
				u32 index = y * TileCountForWidth + x;
				assert(index < tiles.size());
				tiles[index].isSolid = value;
			}

			Vec2f gravity = Vec2f(0, -4);
			Vec2f mouseWorldPos = Vec2f();
			b32 isSinglePlayer = true;

			b32 isEditor = false;
			std::string activeEditorFilePath = std::string("");

			std::vector<Entity> players = std::vector<Entity>();
			std::vector<Wall> walls = std::vector<Wall>();
			std::vector<ControlledPlayer> controlledPlayers = std::vector<ControlledPlayer>();
			std::vector<Tile> tiles = std::vector<Tile>(TileCountForWidth * TileCountForHeight);

			// @Temporary: Remove this later when we have a proper asset system
			Texture texture = {};

			u32 CreatePlayer(const u32 controllerIndex);

			s32 FindControlledPlayerIndex(const u32 controllerIndex);

			void UISaveMap(const bool withDialog);

			void ClearLevel();
			void LoadLevel(const char *filePath);
			void SaveLevel(const char *filePath);
			void CreateWallsFromTiles();

			void HandleControllerConnections(const Input &input);
			void ProcessPlayerInput(const Input &input);
			void MovePlayers(const Input &input);
			void SetExternalForces();
			void EditorUpdate(const Input &input);
		public:
			Game(Renderer &renderer);
			~Game() override;
			void Init() override;
			void Release() override;
			void HandleInput(const Input &input) override;
			void Update(const Input &input) override;
			void Render() override;
		};

	};

};
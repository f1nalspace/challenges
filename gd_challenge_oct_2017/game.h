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
#include "final_game.h"

namespace finalspace {
	inline namespace games {
		namespace mygame {

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

			enum class TileType : s32 {
				None = 0,
				Block,
				Platform,
				Player,
				Enemy,
			};

			struct Wall {
				Vec2f position = {};
				Vec2f ext = {};
				bool isPlatform = false;
			};

			struct ControlledPlayer {
				u32 playerIndex = 0;
				u32 controllerIndex = 0;
			};

			// @NOTE: Down to up
			static const Vec2f TileUVs[] = {
				// None
				Vec2f(0.0f, 0.0f),
				Vec2f(0.0f, 0.0f),

				// Block
				Vec2f(0.5f, 1.0f),
				Vec2f(0.0f, 0.5f),

				// Platform
				Vec2f(1.0f, 1.0f),
				Vec2f(0.5f, 0.5f),

				// Player start
				Vec2f(0.5f, 0.5f),
				Vec2f(0.0f, 0.0f),

				// Enemy start
				Vec2f(1.0f, 0.5f),
				Vec2f(0.5f, 0.0f),
			};

			struct Tile {
				TileType type = TileType::None;
			};

			static constexpr f32 TileSize = 0.5f;
			static constexpr u32 TileCountForWidth = 40;
			static constexpr u32 TileCountForHeight = 22;
			static constexpr f32 GameAspect = TileCountForWidth / (f32)TileCountForHeight;
			static constexpr f32 GameWidth = TileCountForWidth * TileSize;
			static constexpr f32 GameHeight = GameWidth / GameAspect;
			static constexpr f32 HalfGameWidth = GameWidth * 0.5f;
			static constexpr f32 HalfGameHeight = GameHeight * 0.5f;
			static constexpr char MapMagicId[4] = { 'f', 'm', 'a', 'p' };

			struct Game : BaseGame {
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

				// @Temporary: Move to editor file!
				std::string activeEditorFilePath = std::string("");
				bool showSaveDialog = false;
				bool firstTimeSaveDialog = false;
				bool showOpenDialog = false;
				bool firstTimeOpenDialog = false;

				TileType selectedTileType = TileType::None;

				std::vector<Tile> tiles = std::vector<Tile>(TileCountForWidth * TileCountForHeight);

				inline void SetTile(const u32 x, const u32 y, const TileType type) {
					u32 index = y * TileCountForWidth + x;
					assert(index < tiles.size());
					tiles[index].type = type;
				}

				Vec2f gravity = Vec2f(0, -4);
				Vec2f mouseWorldPos = Vec2f();
				bool isSinglePlayer = true;

				bool isEditor = false;

				std::vector<Entity> players = std::vector<Entity>();
				std::vector<Wall> walls = std::vector<Wall>();
				std::vector<ControlledPlayer> controlledPlayers = std::vector<ControlledPlayer>();

				Texture tilesetTexture = {};

				u32 CreatePlayer(const u32 controllerIndex);

				s32 FindControlledPlayerIndex(const u32 controllerIndex);

				void UISaveMap(const bool withDialog);

				void ClearLevel();
				bool LoadMap(const char *filePath);
				void SaveMap(const char *filePath);
				void CreateWallsFromTiles();

				void HandleControllerConnections(const Input &input);
				void ProcessPlayerInput(const Input &input);
				void MovePlayers(const Input &input);
				void SetExternalForces();
				void EditorUpdate();
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
};
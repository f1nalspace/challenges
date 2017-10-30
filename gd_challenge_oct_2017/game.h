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
#include "final_randoms.h"

using namespace fs::maths;
using namespace fs::inputs;
using namespace fs::renderer;
using namespace fs::randoms;

namespace fs {
	namespace games {
		namespace mygame {

			constexpr f32 VecEqualTolerance = 0.001f;
			constexpr f32 EntityPlaceOffset = 0.1f;

			struct Wall;

			struct CollisionState {
				Vec2f normal;
				Wall *wall;
				b32 isColliding;
			};

			constexpr u32 MAX_LAST_COLLISION_COUNT = 8;

			struct AIState {
				enum class Type {
					None = 0,
					DecideDirection,
					ReflectDirection,
					Jump,
					Move,
				};
				Type activeType;
				f32 activeDuration;
				b32 isWaiting;
				f32 waitRemaingTime;
				Type nextType;
				f32 dice;
			};

			struct Entity {
				enum class Type {
					None = 0,
					Player,
					Enemy,
				};

				Vec4f color = Vec4f();
				Vec2f position = Vec2f();
				Vec2f velocity = Vec2f();
				Vec2f acceleration = Vec2f();
				Vec2f ext = Vec2f();
				Vec2f moveDirection = Vec2f();
				Type type = Type::None;
				b32 isGrounded = false;
				f32 horizontalSpeed = 0.0f;
				f32 horizontalDrag = 0.0f;
				b32 canJump = false;
				u32 jumpCount = 0;
				f32 jumpPower = 0.0f;
				CollisionState collisions[4] = {};
				u32 collisionCount;
				AIState ai = {};
			};

			enum class TileType : s32 {
				None = 0,
				Block,
				Platform,
				Player,
				Enemy,
			};

			struct Wall {
				bool isPlatform = false;
				TileType tileType = TileType::None;
				Vec2f position = {};
				Vec2f ext = {};
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

			struct ResultTilePosition {
				b32 found;
				s32 tileX;
				s32 tileY;
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

			typedef u32 PlayerIndex;
			typedef u32 EnemyIndex;

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

				inline const Tile &GetTile(const u32 x, const u32 y) const {
					u32 index = y * TileCountForWidth + x;
					return(tiles[index]);
				}

				Vec2f gravity = Vec2f(0, -4);
				Vec2f mouseWorldPos = Vec2f();
				bool isSinglePlayer = true;

				bool isEditor = false;

				std::vector<Entity> players = std::vector<Entity>();
				std::vector<Entity> enemies = std::vector<Entity>();
				std::vector<Wall> walls = std::vector<Wall>();
				std::vector<ControlledPlayer> controlledPlayers = std::vector<ControlledPlayer>();

				Texture tilesetTexture = {};

				RandomSeries enemyEntropy;

				ResultTilePosition FindFreePlayerTile();

				EnemyIndex CreateEnemy(u32 tileX, u32 tileY);

				PlayerIndex CreatePlayer(const u32 controllerIndex);

				s32 FindControlledPlayerIndex(const u32 controllerIndex);

				void UISaveMap(const bool withDialog);

				void ClearMap();
				bool LoadMap(const char *filePath);
				void SaveMap(const char *filePath);
				void SwitchFromEditorToGame();

				void HandleControllerConnections(const Input &input);
				void ProcessPlayerInput(const Input &input);
				void ProcessEnemyAI(const f32 deltaTime);
				void MoveEntities(std::vector<Entity> &entities, const f32 deltaTime);
				void SetExternalForces();
				void EditorUpdate();
			public:
				Game();
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
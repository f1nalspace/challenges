#pragma once

#define FS_ENABLE_IMGUI 1

#include <vector>
#include <memory>
#include <string>

#include <final_platform_layer.hpp>

#include "final_types.h"
#include "final_maths.h"
#include "final_input.h"
#include "final_renderer.h"
#include "final_game.h"
#include "final_randoms.h"
#include "final_collisions.h"

using namespace fs::maths;
using namespace fs::inputs;
using namespace fs::renderer;
using namespace fs::randoms;
using namespace fs::collisions;

#define TEST_ACTIVE 0
#define TEST_RAYCASTS 1

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

			struct PathNode {
				Vec2f worldPosition;
				Vec2i tilePosition;
				PathNode *closestNodes[8];

				inline bool HasNode(const PathNode *node) {
					for (u32 nodeIndex = 0; nodeIndex < utils::ArrayCount(closestNodes); ++nodeIndex) {
						PathNode *testNode = closestNodes[nodeIndex];
						if (testNode == node) {
							return true;
						}
					}
					return false;
				}
			};

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
				Invalid = 255,
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

			static constexpr f32 TILE_SIZE = 0.5f;
			static constexpr u32 TILE_COUNT_FOR_WIDTH = 40;
			static constexpr u32 TILE_COUNT_FOR_HEIGHT = 22;
			static constexpr f32 GAME_ASPECT = TILE_COUNT_FOR_WIDTH / (f32)TILE_COUNT_FOR_HEIGHT;
			static constexpr f32 GAME_WIDTH = TILE_COUNT_FOR_WIDTH * TILE_SIZE;
			static constexpr f32 GAME_HEIGHT = GAME_WIDTH / GAME_ASPECT;
			static constexpr f32 HALF_GAME_WIDTH = GAME_WIDTH * 0.5f;
			static constexpr f32 HALF_GAME_HEIGHT = GAME_HEIGHT * 0.5f;
			static constexpr char MAP_MAGIC_ID[4] = { 'f', 'm', 'a', 'p' };

			static const Vec2f TILE_EXT = Vec2f(TILE_SIZE, TILE_SIZE) * 0.5f;

			typedef u32 PlayerIndex;
			typedef u32 EnemyIndex;

			struct Game : BaseGame {
				inline Vec2f TileToWorld(const s32 tileX, const s32 tileY) const {
					const Vec2f tileMapExt = Vec2f((f32)TILE_COUNT_FOR_WIDTH * TILE_SIZE, (f32)TILE_COUNT_FOR_HEIGHT * TILE_SIZE) * 0.5f;
					Vec2f result = -tileMapExt +
						Vec2f((f32)tileX, (f32)tileY) * TILE_SIZE +
						TILE_SIZE * 0.5f;
					return(result);
				}
				inline Vec2f TileToWorld(const Vec2i &tilePos) const {
					Vec2f result = TileToWorld(tilePos.x, tilePos.y);
					return(result);
				}

				inline Vec2i WorldToTile(const f32 worldX, const f32 worldY) const {
					const Vec2f tileMapExt = Vec2f((f32)TILE_COUNT_FOR_WIDTH * TILE_SIZE, (f32)TILE_COUNT_FOR_HEIGHT * TILE_SIZE) * 0.5f;
					s32 tileX = (s32)((worldX + tileMapExt.w) / TILE_SIZE);
					s32 tileY = (s32)((worldY + tileMapExt.h) / TILE_SIZE);
					Vec2i result = Vec2i(tileX, tileY);
					return(result);
				}

				inline Vec2i WorldToTile(const Vec2f &worldPos) const {
					Vec2i result = WorldToTile(worldPos.x, worldPos.y);
					return(result);
				}

				inline bool IsSolid(const u32 tileX, const u32 tileY) {
					bool result = false;
					if (IsValidTilePosition(tileX, tileY)) {
						const Tile &tile = GetTile(tileX, tileY);
						result = (tile.type == TileType::Block) || (tile.type == TileType::Platform);
					}
					return(result);
				}
				inline bool IsSolid(const Vec2i &tile) {
					bool result = IsSolid(tile.x, tile.y);
					return(result);
				}

				// @Temporary: Move to editor file!
				std::string activeEditorFilePath = std::string("");
				bool showSaveDialog = false;
				bool firstTimeSaveDialog = false;
				bool showOpenDialog = false;
				bool firstTimeOpenDialog = false;

				TileType selectedTileType = TileType::None;

				std::vector<Tile> tiles = std::vector<Tile>(TILE_COUNT_FOR_WIDTH * TILE_COUNT_FOR_HEIGHT);

				inline void SetTile(const u32 x, const u32 y, const TileType type) {
					u32 index = y * TILE_COUNT_FOR_WIDTH + x;
					assert(index < tiles.size());
					tiles[index].type = type;
				}

				inline bool IsValidTilePosition(const s32 x, const s32 y) {
					bool result = (x >= 0 && x < TILE_COUNT_FOR_WIDTH) && (y >= 0 && y < TILE_COUNT_FOR_HEIGHT);
					return(result);
				}

				inline bool IsValidTilePosition(const Vec2i xy) {
					bool result = IsValidTilePosition(xy.x, xy.y);
					return(result);
				}

				inline const Tile &GetTile(const u32 x, const u32 y) const {
					u32 index = y * TILE_COUNT_FOR_WIDTH + x;
					return(tiles[index]);
				}

				inline const Tile &GetTile(const Vec2i &xy) const {
					const Tile &result = GetTile(xy.x, xy.y);
					return(result);
				}

				inline TileType GetTileType(const u32 x, const u32 y) const {
					u32 index = y * TILE_COUNT_FOR_WIDTH + x;
					return(tiles[index].type);
				}

				Vec2f gravity = Vec2f(0, -4);
				Vec2f mouseWorldPos = Vec2f();
				bool isSinglePlayer = true;

				bool isEditor = false;

				std::vector<Entity> players = std::vector<Entity>();
				std::vector<Entity> enemies = std::vector<Entity>();
				std::vector<Wall> walls = std::vector<Wall>();
				std::vector<PathNode> enemyPath = std::vector<PathNode>();
				std::vector<ControlledPlayer> controlledPlayers = std::vector<ControlledPlayer>();

				Texture tilesetTexture = {};

				RandomSeries enemyEntropy;

			#if TEST_ACTIVE
			#	if TEST_RAYCASTS
				Vec2f rayStart = Vec2f(0, 0) - Vec2f(TILE_SIZE, TILE_SIZE) * 0.5f;
				Vec2f rayEnd = Vec2f(4, -3) - Vec2f(TILE_SIZE, TILE_SIZE) * 0.5f;
			#	endif
			#endif

				LineCastResult DoLineCast(const Ray2D &ray);

				ResultTilePosition FindFreePlayerTile();

				EnemyIndex CreateEnemy(u32 tileX, u32 tileY);

				PlayerIndex CreatePlayer(const u32 controllerIndex);

				s32 FindControlledPlayerIndex(const u32 controllerIndex);

				void UISaveMap(const bool withDialog);

				void ClearMap();
				bool LoadMap(const char *filePath);
				void SaveMap(const char *filePath);
				void Reload();

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
				void Render(const Input &input) override;

			};

		};
	};
};
#pragma once

#include <vector>

#include "final_types.h"
#include "final_maths.h"

using namespace finalspace::maths;

namespace finalspace {
	namespace tiletracer {
		enum class TileTraceDirection {
			Up,
			Right,
			Down,
			Left,
		};

		enum class TileTraceStep {
			None,
			FindStart,
			GetNextOpenTile,
			FindNextTile,
			RotateForward,
			TraverseFindStartingEdge,
			TraverseNextEdge,
			Done,
		};

		struct TileTraceTile {
			b32 isSolid;
			s32 x, y;
			u32 traceDirection;
		};

		struct TileTraceEdge {
			b32 isInvalid;
			s32 index;
			s32 vertIndex0, vertIndex1;
			Vec2i tilePosition;
		};

		struct TileVertices {
			Vec2i verts[4];
		};
		struct TileIndices {
			s32 indices[4];
		};
		struct TileEdges {
			TileTraceEdge edges[4];
			u32 count;
		};

		struct TileTraceChainSegment {
			Vec2i vertices[64];
			u32 vertexCount;
		};

		struct TileTracer {
			Vec2u tileCount;
			std::vector<TileTraceTile> tiles;
			TileTraceStep curStep;
			TileTraceTile *startTile;
			TileTraceTile *curTile;
			TileTraceTile *nextTile;
			TileTraceEdge *startEdge;
			TileTraceEdge *lastEdge;
			TileTraceChainSegment *curChainSegment;
			std::vector<TileTraceTile *> openList;
			std::vector<Vec2i> mainVertices;
			std::vector<TileTraceEdge> mainEdges;
			std::vector<TileTraceChainSegment> chainSegments;

			void Init(const Vec2u &tileCount, u8 *mapTiles);
			bool NextStep();
		};
	};
};
#include "final_tiletrace.h"

#include "final_utils.h"

using namespace finalspace::utils;

namespace finalspace {
	namespace tiletracer {
		/*
		https://en.wikipedia.org/wiki/Moore_neighborhood

		How this algotythm works:

		Find first/next start:
		- Find first solid tile anywhere in the map -> Start-Tile
		- Store this tile as Current-Tile
		- Clear the OpenTile-List

		Add Tile:
		- Mark that Start-Tile as invalid, so we will never process it again
		- Set the initial scan direction for this Start-Tile to Up
		- This Start-Tile is pushed into the OpenTile-List
		- Create all 4 vertices for that tile and push it into the Main-Verts-List, but dont push any vertex which is already in the list
		- Remember each index to the Main-Verts-List you have pushed on or found when already exists
		- Create 4 edges for that tile and store it in a local array with 4 entries long -> Each edge defines the first index and the following index is defined clockwise direction

		  A tile and its vertices and edge indices (Winding order is important!):

				 e1
		  v1 |--------| v2
			 |        |
		  e0 |        | e2
			 |        |
		  v0 |--------| v3
				 e3

		- Loop through all 4 edges and remove edges which overlaps any edge in the Main-Edge-List:
		  A overlap edge is detected when the follow criteria are met: (MainEdge->Index1 == CurrentEdge->Index0) and (MainEdge->Index0 == CurrentEdge->Index1)
		- Add all non-overlapping edges to the Main-Edge-List
		- Get next open tile from the Open-Tile-List, but leave list untouched and continue on "Get next tile forward"
		  or
		  When there is no tile in the Open-Tile-List found we find a new start again, see "Find first/next start":

		Get next tile forward:

		- @TODO: Write down the next steps

		*/

		// Up, Right, Down, Left
		const static Vec2i TILETRACE_DIRECTIONS[] = { Vec2i(0, -1), Vec2i(1, 0), Vec2i(0, 1), Vec2i(-1, 0) };
		const static u64 TILETRACE_DIRECTION_COUNT = ArrayCount(TILETRACE_DIRECTIONS);

		inline TileTraceTile MakeTile(s32 x, s32 y, b32 isSolid) {
			TileTraceTile tile = {};
			tile.x = x;
			tile.y = y;
			tile.isSolid = isSolid;
			return(tile);
		}

		inline TileTraceEdge MakeTileEdge(s32 index, s32 vertIndex0, s32 vertIndex1, const Vec2i &tilePosition) {
			TileTraceEdge result = {};
			result.index = index;
			result.vertIndex0 = vertIndex0;
			result.vertIndex1 = vertIndex1;
			result.tilePosition = tilePosition;
			return(result);
		}

		inline u64 ComputeTileHash(u32 x, u32 y) {
			u64 result = ((u64)x << 32) | (u64)y;
			return(result);
		}

		inline u32 ComputeTileIndex(const Vec2u &dimension, u32 x, u32 y) {
			u32 result = y * dimension.w + x;
			return(result);
		}

		inline b32 IsTileSolid(std::vector<TileTraceTile> &tiles, const Vec2u &dimension, s32 x, s32 y) {
			b32 result = false;
			if ((x >= 0 && x < (s32)dimension.w) && (y >= 0 && y < (s32)dimension.h)) {
				u32 tileIndex = ComputeTileIndex(dimension, x, y);
				assert(tileIndex < dimension.w * dimension.h);
				result = tiles[tileIndex].isSolid > 0;
			}
			return(result);
		}

		inline TileTraceTile *GetTile(std::vector<TileTraceTile> &tiles, const Vec2u &dimension, u32 x, u32 y) {
			assert((x < (u32)dimension.w) && (y < (u32)dimension.h));
			u32 tileIndex = ComputeTileIndex(dimension, x, y);
			return &tiles[tileIndex];
		}

		inline void SetTileSolid(std::vector<TileTraceTile> &tiles, const Vec2u &dimension, u32 x, u32 y, b32 value) {
			assert((x < (u32)dimension.w) && (y < (u32)dimension.h));
			u32 tileIndex = ComputeTileIndex(dimension, x, y);
			tiles[tileIndex].isSolid = value;
		}
		inline void RemoveTile(std::vector<TileTraceTile> &tiles, const Vec2u &dimension, u32 x, u32 y) {
			SetTileSolid(tiles, dimension, x, y, -1);
		}

		static void CopyTiles(const std::vector<TileTraceTile> &sourceTiles, TileTraceTile *destTiles, const Vec2u &dimension) {
			u32 tileCount = dimension.w * dimension.h;
			for (u32 index = 0; index < tileCount; ++index) {
				destTiles[index] = sourceTiles[index];
			}
		}

		static TileTraceTile *GetFirstSolidTile(std::vector<TileTraceTile> &tiles, const Vec2u &dimension) {
			for (u32 tileY = 0; tileY < dimension.h; ++tileY) {
				for (u32 tileX = 0; tileX < dimension.w; ++tileX) {
					if (IsTileSolid(tiles, dimension, tileX, tileY)) {
						TileTraceTile *tile = GetTile(tiles, dimension, tileX, tileY);
						return tile;
					}
				}
			}
			return nullptr;
		}

		static TileVertices CreateTileVertices(TileTraceTile *tile) {
			TileVertices result = {};
			result.verts[0] = Vec2i(tile->x, tile->y + 1);
			result.verts[1] = Vec2i(tile->x, tile->y);
			result.verts[2] = Vec2i(tile->x + 1, tile->y);
			result.verts[3] = Vec2i(tile->x + 1, tile->y + 1);
			return(result);
		}

		static TileIndices PushTileVertices(TileTracer &tracer, TileTraceTile *tile) {
			TileIndices result = {};
			TileVertices tileVerts = CreateTileVertices(tile);
			assert(ArrayCount(tileVerts.verts) == ArrayCount(result.indices));
			for (u32 vertIndex = 0; vertIndex < ArrayCount(tileVerts.verts); ++vertIndex) {
				Vec2i vertex = tileVerts.verts[vertIndex];
				s32 matchedMainVertexIndex = -1;
				for (u32 mainVertexIndex = 0; mainVertexIndex < tracer.mainVertices.size(); ++mainVertexIndex) {
					if (IsEqual(tracer.mainVertices[mainVertexIndex], vertex)) {
						matchedMainVertexIndex = mainVertexIndex;
						break;
					}
				}
				if (matchedMainVertexIndex == -1) {
					result.indices[vertIndex] = (s32)tracer.mainVertices.size();
					tracer.mainVertices.push_back(vertex);
				} else {
					result.indices[vertIndex] = matchedMainVertexIndex;
				}
			}
			return(result);
		}

		static TileEdges CreateTileEdges(TileIndices tileIndices, const Vec2u &tileCount, TileTraceTile *tile) {
			TileEdges result = {};
			u64 indexCount = utils::ArrayCount(tileIndices.indices);
			for (u32 index = 0; index < indexCount; ++index) {
				s32 e0 = tileIndices.indices[index];
				s32 e1 = tileIndices.indices[(index + 1) % indexCount];
				result.edges[result.count++] = MakeTileEdge(index, e0, e1, Vec2i(tile->x, tile->y));
			}
			return(result);
		}

		static TileEdges RemoveOverlapEdges(TileTracer &tracer, TileEdges inputEdges) {
			TileEdges result = {};
			for (u32 edgeIndex = 0; edgeIndex < inputEdges.count; ++edgeIndex) {
				TileTraceEdge inputEdge = inputEdges.edges[edgeIndex];
				bool addIt = true;
				for (u32 mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
					TileTraceEdge mainEdge = tracer.mainEdges[mainEdgeIndex];
					if (inputEdge.vertIndex0 == mainEdge.vertIndex1 && inputEdge.vertIndex1 == mainEdge.vertIndex0) {
						addIt = false;
						tracer.mainEdges.erase(tracer.mainEdges.begin() + mainEdgeIndex);
						break;
					}
				}
				if (addIt) {
					result.edges[result.count++] = inputEdge;
				}
			}
			return(result);
		}

		static bool IsTileSharedCommonEdges(TileTracer &tracer, TileVertices tileVertices) {
			u64 vertexCount = ArrayCount(tileVertices.verts);
			for (u32 vertIndex = 0; vertIndex < vertexCount; ++vertIndex) {
				Vec2i tv0 = tileVertices.verts[vertIndex];
				Vec2i tv1 = tileVertices.verts[(vertIndex + 1) % vertexCount];
				for (u32 edgeIndex = 0; edgeIndex < tracer.mainEdges.size(); ++edgeIndex) {
					TileTraceEdge edge = tracer.mainEdges[edgeIndex];
					Vec2i mv0 = tracer.mainVertices[edge.vertIndex0];
					Vec2i mv1 = tracer.mainVertices[edge.vertIndex1];
					if (IsEqual(tv0, mv1) && IsEqual(tv1, mv0)) {
						return true;
					}
				}
			}
			return false;
		}

		inline void RemoveSegmentVertex(TileTraceChainSegment *chainSegment, u32 index) {
			utils::ArrayRemoveAndKeepOrder<Vec2i, u32>(chainSegment->vertices, index, chainSegment->vertexCount);
		}

		static void ClearLineSegmentPoints(TileTraceChainSegment *segment, u32 firstIndex, u32 middleIndex, u32 lastIndex) {
			Vec2i last = segment->vertices[lastIndex];
			Vec2i middle = segment->vertices[middleIndex];
			Vec2i first = segment->vertices[firstIndex];
			Vec2i d1 = last - middle;
			Vec2i d2 = middle - first;
			s32 d = Dot(d1, d2);
			if (d > 0) {
				RemoveSegmentVertex(segment, middleIndex);
			}
		}

		static void OptimizeChainSegment(TileTraceChainSegment *segment) {
			if (segment->vertexCount > 2) {
				u32 lastIndex = segment->vertexCount - 1;
				ClearLineSegmentPoints(segment, lastIndex - 2, lastIndex - 1, lastIndex);
			}
		}

		static void FinalizeChainSegment(TileTraceChainSegment *chainSegment) {
			if (chainSegment->vertexCount > 2) {
				ClearLineSegmentPoints(chainSegment, chainSegment->vertexCount - 1, 0, 1);
			}
			if (chainSegment->vertexCount > 2) {
				ClearLineSegmentPoints(chainSegment, 0, chainSegment->vertexCount - 1, chainSegment->vertexCount - 2);
			}
		}

		inline void AddChainSegmentVertex(TileTraceChainSegment *chainSegment, const Vec2i &vertex) {
			assert(chainSegment->vertexCount < ArrayCount(chainSegment->vertices));
			chainSegment->vertices[chainSegment->vertexCount++] = vertex;
		}

		static bool ProcessTraverseNextEdge(TileTracer &tracer) {
			for (u32 mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
				TileTraceEdge *curEdge = &tracer.mainEdges[mainEdgeIndex];
				if (!curEdge->isInvalid && (curEdge->vertIndex0 == tracer.lastEdge->vertIndex1)) {
					// If v0 from current edge equals starting edge - then we are finished
					if (curEdge->vertIndex1 == tracer.startEdge->vertIndex0) {
						// We are done with this line segment - Set cur step to find next starting edge
						tracer.lastEdge = nullptr;
						tracer.curStep = TileTraceStep::TraverseFindStartingEdge;
						// Optimize and finalize shape
						OptimizeChainSegment(tracer.curChainSegment);
						FinalizeChainSegment(tracer.curChainSegment);
						// Add list vertex to the end again, because we have a fully closed chain
						AddChainSegmentVertex(tracer.curChainSegment, tracer.curChainSegment->vertices[0]);
					} else {
						// Now our current edge is the last edge
						tracer.lastEdge = curEdge;
						// Add always the first edge vertex to the list
						AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[curEdge->vertIndex1]);
						// Optimize shape
						OptimizeChainSegment(tracer.curChainSegment);
					}
					curEdge->isInvalid = true;
					return true;
				}
			}

			// We will come here for a line segment which is not fully closed, may have holes or something
			if (tracer.curChainSegment->vertexCount > 0) {
				// We are done with this line segment - Set cur step to find next starting edge
				tracer.lastEdge = nullptr;
				tracer.curStep = TileTraceStep::TraverseFindStartingEdge;
				// Optimize and finalize shape
				OptimizeChainSegment(tracer.curChainSegment);
				FinalizeChainSegment(tracer.curChainSegment);
				return true;
			}

			// We are totally done, there is no line segment or anything to do
			// @TODO: An assert would be appropiate here
			tracer.curStep = TileTraceStep::Done;
			return false;
		}

		static bool ProcessTraverseFindStartingEdge(TileTracer &tracer) {
			bool result = false;

			// Find next free starting edge - at the start this is always null
			tracer.startEdge = nullptr;
			s32 startEdgeIndex = -1;
			for (u32 mainEdgeIndex = 0; mainEdgeIndex < tracer.mainEdges.size(); ++mainEdgeIndex) {
				TileTraceEdge *mainEdge = &tracer.mainEdges[mainEdgeIndex];
				if (!mainEdge->isInvalid) {
					startEdgeIndex = mainEdgeIndex;
					tracer.startEdge = mainEdge;
					break;
				}
			}

			if (tracer.startEdge != nullptr) {
				// Continue when we found a starting edge and create the actual line segment for it
				result = true;
				tracer.lastEdge = tracer.startEdge;
				tracer.mainEdges[startEdgeIndex].isInvalid = true;
				tracer.curStep = TileTraceStep::TraverseNextEdge;
				tracer.chainSegments.push_back({});
				tracer.curChainSegment = &tracer.chainSegments[tracer.chainSegments.size() - 1];
				AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[tracer.startEdge->vertIndex0]);
				AddChainSegmentVertex(tracer.curChainSegment, tracer.mainVertices[tracer.startEdge->vertIndex1]);
			} else {
				// We are completely done
				tracer.curStep = TileTraceStep::Done;
			}

			return(result);
		}

		void TileTracer::Init(const Vec2u &tileCount, u8 *mapTiles) {
			this->tileCount = tileCount;
			tiles.reserve(tileCount.x  * tileCount.y);
			for (u32 tileY = 0; tileY < tileCount.h; ++tileY) {
				for (u32 tileX = 0; tileX < tileCount.w; ++tileX) {
					u32 tileIndex = ComputeTileIndex(tileCount, tileX, tileY);
					b32 isSolid = mapTiles[tileIndex] > 0;
					TileTraceTile *tile = &tiles[tileIndex];
					*tile = MakeTile(tileX, tileY, isSolid);
				}
			}

			curStep = TileTraceStep::FindStart;
			openList.clear();
			startTile = nullptr;
			mainVertices.clear();
			mainEdges.clear();
			chainSegments.clear();

			curTile = nullptr;
			nextTile = nullptr;
		}

		static void GetNextOpenTile(TileTracer &tracer) {
			if (tracer.openList.size() > 0) {
				tracer.curTile = tracer.openList[tracer.openList.size() - 1];
				tracer.curStep = TileTraceStep::FindNextTile;
			} else {
				tracer.curStep = TileTraceStep::FindStart;
			}
		}

		static void RotateForward(TileTracer &tracer) {
			if (tracer.curTile->traceDirection < (TILETRACE_DIRECTION_COUNT - 1)) {
				++tracer.curTile->traceDirection;
				tracer.curStep = TileTraceStep::FindNextTile;
			} else {
				tracer.openList.pop_back();
				tracer.curStep = TileTraceStep::GetNextOpenTile;
				GetNextOpenTile(tracer);
			}
		}

		static void AddTile(TileTracer &tracer, TileTraceTile *tile) {
			// Add the start tile to the open list and remove it from the map
			tracer.openList.push_back(tile);
			RemoveTile(tracer.tiles, tracer.tileCount, tile->x, tile->y);

			// Create tile vertices/indices and edges for the next tile
			TileIndices tileIndices = PushTileVertices(tracer, tile);
			TileEdges tileEdges = CreateTileEdges(tileIndices, tracer.tileCount, tile);

			// Remove edges that overlap from the main edge list and the edge list for NextTile
			tileEdges = RemoveOverlapEdges(tracer, tileEdges);

			// Push the remaining edges to the main edges list
			for (u32 tileEdgeIndex = 0; tileEdgeIndex < tileEdges.count; ++tileEdgeIndex) {
				tracer.mainEdges.push_back(tileEdges.edges[tileEdgeIndex]);
			}
		}

		bool TileTracer::NextStep() {
			bool result = true;
			switch (curStep) {
				case TileTraceStep::Done:
				{
					result = false;
				}; break;
				case TileTraceStep::TraverseNextEdge:
				{
					result = ProcessTraverseNextEdge(*this);
				}; break;
				case TileTraceStep::TraverseFindStartingEdge:
				{
					result = ProcessTraverseFindStartingEdge(*this);
				}; break;
				case TileTraceStep::FindStart:
				{
					openList.clear();
					curTile = nullptr;
					startTile = GetFirstSolidTile(tiles, tileCount);
					if (startTile != nullptr) {
						// Add the start tile to the open list and build vertices and edges from it
						AddTile(*this, startTile);

						// Set next step to get open tile and process it immediatitly
						curStep = TileTraceStep::GetNextOpenTile;
						GetNextOpenTile(*this);
					} else {
						// No start found, exit if we have not found any edges at all
						if (mainEdges.size() == 0) {
							result = false;
						} else {
							// Clear all chain segments
							chainSegments.clear();
							curStep = TileTraceStep::TraverseFindStartingEdge;
						}
					}
				}; break;
				case TileTraceStep::GetNextOpenTile:
				{
					GetNextOpenTile(*this);
				}; break;
				case TileTraceStep::FindNextTile:
				{
					assert(curTile != nullptr);

					// Tile in the "forward" direction of the current tile
					s32 nx = curTile->x + TILETRACE_DIRECTIONS[curTile->traceDirection].x;
					s32 ny = curTile->y + TILETRACE_DIRECTIONS[curTile->traceDirection].y;
					nextTile = IsTileSolid(tiles, tileCount, nx, ny) ? GetTile(tiles, tileCount, nx, ny) : nullptr;

					if (nextTile != nullptr) {
						// Create tile vertices for next tile and check if it shares common edges
						TileVertices tileVertices = CreateTileVertices(nextTile);
						bool sharesCommonEdge = IsTileSharedCommonEdges(*this, tileVertices);
						if (sharesCommonEdge) {
							// Add the next tile to the open list and build vertices and edges from it
							AddTile(*this, nextTile);

							// Set next step to get open tile and process it immediatitly
							curStep = TileTraceStep::GetNextOpenTile;
							GetNextOpenTile(*this);
						} else {
							curStep = TileTraceStep::RotateForward;
							RotateForward(*this);
						}
					} else {
						curStep = TileTraceStep::RotateForward;
						RotateForward(*this);
					}
				}; break;
				case TileTraceStep::RotateForward:
				{
					RotateForward(*this);
				}; break;
				default:
					assert(!"Invalid code path!");
					break;
			}
			return(result);
		}
	}
}
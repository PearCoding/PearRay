#pragma once

#include "renderer/RenderEnums.h"
#include "renderer/RenderStatistics.h"

#include <tbb/queuing_rw_mutex.h>
#include <vector>

namespace PR {

class RenderContext;
class RenderTile;
class RenderThread;
class PR_LIB_CORE RenderTileMap {
public:
	RenderTileMap();
	~RenderTileMap();

	inline size_t tileCount() const { return mTiles.size(); }
	inline Size2i maxTileSize() const { return mMaxTileSize; }

	void init(RenderContext* context, uint32 rtx, uint32 rty, TileMode mode);

	/// Split tiles to minimize workoverload on single tiles
	void optimize();

	RenderTile* getNextTile(const RenderThread* thread);
	bool allFinished() const;
	void reset();
	/// Unmark all tiles to prepare for next linear iteration
	void makeAllIdle();

	RenderStatistics statistics() const;
	double percentage() const;

private:
	void clearMap();

	Size2i mMaxTileSize;
	std::vector<std::unique_ptr<RenderTile>> mTiles;

	using Mutex = tbb::queuing_rw_mutex;
	mutable Mutex mMutex;
};
} // namespace PR

#pragma once

#include "renderer/RenderEnums.h"
#include "renderer/RenderTileStatistics.h"

#include <tbb/queuing_rw_mutex.h>
#include <vector>

namespace PR {

class RenderContext;
class RenderTile;
class PR_LIB_CORE RenderTileMap {
public:
	RenderTileMap();
	~RenderTileMap();

	inline size_t tileCount() const { return mTileMap.size(); }
	inline Size2i maxTileSize() const { return mMaxTileSize; }

	void init(const RenderContext& context, uint32 rtx, uint32 rty, TileMode mode);

	// Split tiles to minimize workoverload on single tiles
	void optimize();

	RenderTile* getNextTile(uint32 maxIter);
	bool allFinished() const;
	void reset();

	RenderTileStatistics statistics() const;
	double percentage() const;

private:
	void clearMap();

	Size2i mMaxTileSize;
	std::vector<RenderTile*> mTileMap;

	using Mutex = tbb::queuing_rw_mutex;
	mutable Mutex mMutex;
};
} // namespace PR

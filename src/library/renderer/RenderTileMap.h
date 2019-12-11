#pragma once

#include "renderer/RenderEnums.h"
#include "renderer/RenderTileStatistics.h"

#include <vector>

namespace PR {

class RenderContext;
class RenderTile;
class PR_LIB RenderTileMap {
public:
	RenderTileMap(uint32 xcount, uint32 ycount, uint32 tilewidth, uint32 tileheight);
	~RenderTileMap();

	inline size_t tileCount() const { return mTileMap.size(); }
	inline uint32 tileXCount() const { return mTileXCount; }
	inline uint32 tileYCount() const { return mTileYCount; }
	inline uint32 tileWidth() const { return mTileWidth; }
	inline uint32 tileHeight() const { return mTileHeight; }

	void init(const RenderContext& context, TileMode mode);

	RenderTile* getNextTile(uint32 maxSample);
	void reset();

	RenderTileStatistics statistics() const;

private:
	uint32 mTileXCount;
	uint32 mTileYCount;
	uint32 mTileWidth;
	uint32 mTileHeight;
	std::vector<RenderTile*> mTileMap;
};
} // namespace PR

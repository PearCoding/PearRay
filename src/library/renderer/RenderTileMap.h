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
}

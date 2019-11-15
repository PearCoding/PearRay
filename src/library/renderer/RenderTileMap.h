#pragma once

#include "renderer/RenderEnums.h"
#include "renderer/RenderTileStatistics.h"

#include <vector>

namespace PR {

class RenderContext;
class RenderTile;
class PR_LIB RenderTileMap {
public:
	RenderTileMap(size_t xcount, size_t ycount, size_t tilewidth, size_t tileheight);
	~RenderTileMap();

	inline size_t tileCount() const { return mTileMap.size(); }

	void init(const RenderContext& context, TileMode mode);

	RenderTile* getNextTile(uint32 maxSample);
	void reset();

	RenderTileStatistics statistics() const;

private:
	size_t mTileXCount;
	size_t mTileYCount;
	size_t mTileWidth;
	size_t mTileHeight;
	std::vector<RenderTile*> mTileMap;
};
}

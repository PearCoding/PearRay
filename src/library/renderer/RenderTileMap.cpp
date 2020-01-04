#include "RenderTileMap.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "math/Generator.h"

#include "Logger.h"
namespace PR {
RenderTileMap::RenderTileMap(uint32 xcount, uint32 ycount, uint32 tilewidth, uint32 tileheight)
	: mTileXCount(xcount)
	, mTileYCount(ycount)
	, mTileWidth(tilewidth)
	, mTileHeight(tileheight)
	, mTileMap()
{
}

RenderTileMap::~RenderTileMap()
{
	for (RenderTile* tile : mTileMap) {
		if (tile)
			delete tile;
	}
}

void RenderTileMap::init(const RenderContext& context, TileMode mode)
{
	PR_PROFILE_THIS;

	// Clear previous data
	for (RenderTile* tile : mTileMap) {
		if (tile)
			delete tile;
	}
	mTileMap.clear();

	// New data
	mTileMap.resize(mTileXCount * static_cast<size_t>(mTileYCount), nullptr);

	switch (mode) {
	default:
	case TM_LINEAR:
		for (uint32 i = 0; i < mTileYCount; ++i) {
			for (uint32 j = 0; j < mTileXCount; ++j) {
				uint32 sx = j * mTileWidth;
				uint32 sy = i * mTileHeight;

				if (sx < context.width() - 1
					&& sy < context.height() - 1) {
					mTileMap[i * mTileXCount + j] = new RenderTile(
						sx,
						sy,
						std::min<uint32>(context.width(), sx + mTileWidth),
						std::min<uint32>(context.height(), sy + mTileHeight),
						context, i * mTileXCount + j);
				} else {
					mTileMap[i * mTileXCount + j] = nullptr;
				}
			}
		}
		break;
	case TM_TILE: {
		uint32 k = 0;
		// Even
		for (uint32 i = 0; i < mTileYCount; ++i) {
			for (uint32 j = ((i % 2) ? 1 : 0); j < mTileXCount; j += 2) {
				uint32 sx = j * mTileWidth;
				uint32 sy = i * mTileHeight;

				if (sx < context.width() - 1
					&& sy < context.height() - 1) {
					mTileMap[k] = new RenderTile(
						sx,
						sy,
						std::min<uint32>(context.width(), sx + mTileWidth),
						std::min<uint32>(context.height(), sy + mTileHeight),
						context, k);
				} else {
					mTileMap[k] = nullptr;
				}
				++k;
			}
		}
		// Odd
		for (uint32 i = 0; i < mTileYCount; ++i) {
			for (uint32 j = ((i % 2) ? 0 : 1); j < mTileXCount; j += 2) {
				uint32 sx = j * mTileWidth;
				uint32 sy = i * mTileHeight;

				if (sx < context.width() - 1
					&& sy < context.height() - 1) {
					mTileMap[k] = new RenderTile(
						sx,
						sy,
						std::min<uint32>(context.width(), sx + mTileWidth),
						std::min<uint32>(context.height(), sy + mTileHeight),
						context, k);
				} else {
					mTileMap[k] = nullptr;
				}
				++k;
			}
		}
	} break;
	case TM_SPIRAL: {
		MinRadiusGenerator<2> generator(std::max(mTileXCount / 2, mTileYCount / 2));
		uint32 i = 0;
		while (generator.hasNext()) {
			const auto p  = generator.next();
			const auto tx = mTileXCount / 2 + p[0];
			const auto ty = mTileYCount / 2 + p[1];

			if (tx < mTileXCount && ty < mTileYCount) {
				if (tx * mTileWidth < context.width() - 1
					&& ty * mTileHeight < context.height() - 1) {
					mTileMap[i] = new RenderTile(
						tx * mTileWidth,
						ty * mTileHeight,
						std::min<uint32>(context.width(), tx * mTileWidth + mTileWidth),
						std::min<uint32>(context.height(), ty * mTileHeight + mTileHeight),
						context, i);
				} else {
					mTileMap[i] = nullptr;
				}
				++i;
			}
		}
	} break;
	}
}

RenderTile* RenderTileMap::getNextTile(uint32 maxIter)
{
	PR_PROFILE_THIS;

	for (uint32 i = 0; i < mTileYCount; ++i) {
		for (uint32 j = 0; j < mTileXCount; ++j) {
			auto tile = mTileMap[i * mTileXCount + j];
			if (tile && tile->iterationCount() < maxIter && !tile->isFinished()) {
				if (tile->accuire())
					return tile;
			}
		}
	}

	return nullptr;
}

bool RenderTileMap::allFinished() const
{
	PR_PROFILE_THIS;

	for (uint32 i = 0; i < mTileYCount; ++i) {
		for (uint32 j = 0; j < mTileXCount; ++j) {
			auto tile = mTileMap[i * mTileXCount + j];
			if (tile && (!tile->isFinished() || tile->isWorking()))
				return false;
		}
	}
	return true;
}

RenderTileStatistics RenderTileMap::statistics() const
{
	PR_PROFILE_THIS;

	RenderTileStatistics s;
	for (uint32 i = 0; i < mTileYCount; ++i) {
		for (uint32 j = 0; j < mTileXCount; ++j) {
			if (mTileMap[i * mTileXCount + j])
				s += mTileMap[i * mTileXCount + j]->statistics();
		}
	}
	return s;
}

float RenderTileMap::percentage() const
{
	PR_PROFILE_THIS;

	uint32 maxSamples	  = 0;
	uint32 samplesRendered = 0;
	for (uint32 i = 0; i < mTileYCount; ++i) {
		for (uint32 j = 0; j < mTileXCount; ++j) {
			auto tile = mTileMap[i * mTileXCount + j];
			if (tile) {
				maxSamples += tile->maxPixelSamples();
				samplesRendered += tile->pixelSamplesRendered();
			}
		}
	}

	if (maxSamples == 0)
		return 1.0f;
	else
		return samplesRendered / (float)maxSamples;
}

void RenderTileMap::reset()
{
	PR_PROFILE_THIS;

	for (uint32 i = 0; i < mTileYCount; ++i)
		for (uint32 j = 0; j < mTileXCount; ++j)
			if (mTileMap[i * mTileXCount + j])
				mTileMap[i * mTileXCount + j]->reset();
}
} // namespace PR

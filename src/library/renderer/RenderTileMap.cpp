#include "RenderTileMap.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "math/Generator.h"

#include "Logger.h"
namespace PR {
RenderTileMap::RenderTileMap()
	: mTileMap()
{
}

RenderTileMap::~RenderTileMap()
{
	clearMap();
}

void RenderTileMap::clearMap()
{
	Mutex::scoped_lock lock(mMutex, true);
	for (auto tile : mTileMap)
		delete tile;

	mTileMap.clear();
}

void RenderTileMap::init(const RenderContext& context, uint32 rtx, uint32 rty, TileMode mode)
{
	PR_PROFILE_THIS;

	mMaxTileSize		= context.viewSize();
	mMaxTileSize.Width	= std::max(2, mMaxTileSize.Width / (int)rtx);
	mMaxTileSize.Height = std::max(2, mMaxTileSize.Height / (int)rty);

	const Size1i tx = std::max<int32>(2, std::ceil(context.viewSize().Width / (float)mMaxTileSize.Width));
	const Size1i ty = std::max<int32>(2, std::ceil(context.viewSize().Height / (float)mMaxTileSize.Height));

	// Clear previous data
	clearMap();

	// New data
	mTileMap.reserve(tx * ty);
	auto addTile = [&](Point1i sx, Point1i sy) {
		Point2i start = Point2i(sx, sy);
		Point2i end	  = (start + mMaxTileSize).cwiseMin(Point2i(context.viewSize().Width, context.viewSize().Height));
		auto tile	  = new RenderTile(start, end, context);
		mTileMap.emplace_back(tile);
	};

	switch (mode) {
	default:
	case TM_LINEAR:
		for (Point1i i = 0; i < ty; ++i) {
			for (Point1i j = 0; j < tx; ++j) {
				Point1i sx = j * mMaxTileSize.Width;
				Point1i sy = i * mMaxTileSize.Height;

				if (sx < context.viewSize().Width - 1
					&& sy < context.viewSize().Height - 1)
					addTile(sx, sy);
			}
		}
		break;
	case TM_TILE:
		// Even
		for (Point1i i = 0; i < ty; ++i) {
			for (Point1i j = ((i % 2) ? 1 : 0); j < tx; j += 2) {
				Point1i sx = j * mMaxTileSize.Width;
				Point1i sy = i * mMaxTileSize.Height;

				if (sx < context.viewSize().Width - 1
					&& sy < context.viewSize().Height - 1)
					addTile(sx, sy);
			}
			// Odd
			for (Point1i i = 0; i < ty; ++i) {
				for (Point1i j = ((i % 2) ? 0 : 1); j < tx; j += 2) {
					Point1i sx = j * mMaxTileSize.Width;
					Point1i sy = i * mMaxTileSize.Height;

					if (sx < context.viewSize().Width - 1
						&& sy < context.viewSize().Height - 1)
						addTile(sx, sy);
				}
			}
		}
		break;
	case TM_SPIRAL: {
		MinRadiusGenerator<2> generator(std::max(tx / 2, ty / 2));
		while (generator.hasNext()) {
			const auto p   = generator.next();
			const auto itx = tx / 2 + p[0];
			const auto ity = ty / 2 + p[1];

			if (itx < tx && ity < ty) {
				if (itx * mMaxTileSize.Width < context.viewSize().Width - 1
					&& ity * mMaxTileSize.Height < context.viewSize().Height - 1)
					addTile(itx * mMaxTileSize.Width,
							ity * mMaxTileSize.Height);
			}
		}
	} break;
	}
}

RenderTile* RenderTileMap::getNextTile(uint32 maxIter)
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (auto tile : mTileMap)
		if (tile && tile->iterationCount() < maxIter && !tile->isFinished())
			if (tile->accuire())
				return tile;

	return nullptr;
}

bool RenderTileMap::allFinished() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (auto tile : mTileMap)
		if (!tile->isFinished() || tile->isWorking())
			return false;
	return true;
}

RenderTileStatistics RenderTileMap::statistics() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	RenderTileStatistics s;
	for (auto tile : mTileMap)
		s += tile->statistics();
	return s;
}

float RenderTileMap::percentage() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	uint32 maxSamples	   = 0;
	uint32 samplesRendered = 0;
	for (auto tile : mTileMap) {
		maxSamples += tile->maxPixelSamples();
		samplesRendered += tile->pixelSamplesRendered();
	}

	if (maxSamples == 0)
		return 1.0f;
	else
		return samplesRendered / (float)maxSamples;
}

void RenderTileMap::reset()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (auto tile : mTileMap)
		tile->reset();
}

void RenderTileMap::optimize()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, true);
	constexpr Size1i MinTileSize = 8;
	constexpr int64 MinTimeSpent = 5 * 1000 * 1000; // 5s

	int64 fullWorkTime = 0;
	int64 validTiles   = 0;
	for (auto tile : mTileMap) {
		if (tile->lastWorkTime().count() > 0) {
			fullWorkTime += tile->lastWorkTime().count();
			++validTiles;
		}
	}

	if (fullWorkTime == 0)
		return;

	const int64 thrWorkTime = std::max(MinTimeSpent, (int64)std::floor(fullWorkTime / (double)validTiles));
	for (auto it = mTileMap.begin(); it != mTileMap.end();) {
		auto ms = (*it)->lastWorkTime().count();
		if (ms < thrWorkTime || (*it)->isFinished()) {
			++it;
			continue;
		}

		auto tile = *it;

		// Split tile at the largest dimension
		const Size2i tileSize = tile->viewSize();
		const int largeDim	  = (tileSize.Width >= tileSize.Height) ? 0 : 1;

		if (tileSize.asArray()(largeDim) <= MinTileSize) {
			++it;
			continue;
		}

		auto tiles = tile->split(largeDim);

		PR_LOG(L_DEBUG) << "Split Tile " << PR_FMT_MAT(tileSize.asArray())
						<< " into " << PR_FMT_MAT(tiles.first->viewSize().asArray())
						<< " | " << PR_FMT_MAT(tiles.second->viewSize().asArray()) << std::endl;

		delete tile;
		it = mTileMap.insert(mTileMap.erase(it), { tiles.first, tiles.second });
		std::advance(it, 2);
	}
}
} // namespace PR

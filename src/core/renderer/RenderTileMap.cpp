#include "RenderTileMap.h"
#include "Logger.h"
#include "Profiler.h"
#include "RenderContext.h"
#include "RenderTile.h"
#include "math/Bits.h"
#include "math/Generator.h"

namespace PR {
RenderTileMap::RenderTileMap()
	: mTiles()
{
}

RenderTileMap::~RenderTileMap()
{
	clearMap();
}

void RenderTileMap::clearMap()
{
	Mutex::scoped_lock lock(mMutex, true);
	mTiles.clear();
}

void RenderTileMap::init(RenderContext* context, uint32 rtx, uint32 rty, TileMode mode)
{
	PR_PROFILE_THIS;

	mMaxTileSize		= context->viewSize();
	mMaxTileSize.Width	= std::max<int32>(2, std::floor(mMaxTileSize.Width / (float)rtx));
	mMaxTileSize.Height = std::max<int32>(2, std::floor(mMaxTileSize.Height / (float)rty));

	const Size1i tx = std::ceil(context->viewSize().Width / (float)mMaxTileSize.Width);
	const Size1i ty = std::ceil(context->viewSize().Height / (float)mMaxTileSize.Height);

	// Clear previous data
	clearMap();

	// New data
	PR_ASSERT(tx * ty > 0, "Expected positive tile size");
	mTiles.reserve(static_cast<size_t>(tx * ty));
	auto addTile = [&](Point1i sx, Point1i sy) {
		Point2i start = Point2i(sx, sy);
		Point2i end	  = (start + mMaxTileSize).cwiseMin(Point2i(context->viewSize().Width, context->viewSize().Height));
		mTiles.emplace_back(std::make_unique<RenderTile>(start, end, context));
	};

	switch (mode) {
	default:
	case TileMode::Linear:
		for (Point1i i = 0; i < ty; ++i) {
			for (Point1i j = 0; j < tx; ++j) {
				Point1i sx = j * mMaxTileSize.Width;
				Point1i sy = i * mMaxTileSize.Height;

				if (sx < context->viewSize().Width - 1
					&& sy < context->viewSize().Height - 1)
					addTile(sx, sy);
			}
		}
		break;
	case TileMode::Tile:
		// Even
		for (Point1i i = 0; i < ty; ++i) {
			for (Point1i j = ((i % 2) ? 1 : 0); j < tx; j += 2) {
				Point1i sx = j * mMaxTileSize.Width;
				Point1i sy = i * mMaxTileSize.Height;

				if (sx < context->viewSize().Width - 1
					&& sy < context->viewSize().Height - 1)
					addTile(sx, sy);
			}
			// Odd
			for (Point1i i = 0; i < ty; ++i) {
				for (Point1i j = ((i % 2) ? 0 : 1); j < tx; j += 2) {
					Point1i sx = j * mMaxTileSize.Width;
					Point1i sy = i * mMaxTileSize.Height;

					if (sx < context->viewSize().Width - 1
						&& sy < context->viewSize().Height - 1)
						addTile(sx, sy);
				}
			}
		}
		break;
	case TileMode::Spiral: {
		MinRadiusGenerator<2> generator(std::max(tx / 2, ty / 2));
		while (generator.hasNext()) {
			const auto p   = generator.next();
			const auto itx = tx / 2 + p[0];
			const auto ity = ty / 2 + p[1];

			if (itx < tx && ity < ty) {
				if (itx * mMaxTileSize.Width < context->viewSize().Width - 1
					&& ity * mMaxTileSize.Height < context->viewSize().Height - 1)
					addTile(itx * mMaxTileSize.Width,
							ity * mMaxTileSize.Height);
			}
		}
	} break;
	case TileMode::ZOrder: {
		uint64 i = 0;
		int32 c	 = 0;
		while (c < tx * ty) {
			uint32 x, y;
			morton_2_xy(i, x, y);
			++i;
			if (x >= (uint32)tx || y >= (uint32)ty)
				continue;

			++c;
			Point1i sx = x * mMaxTileSize.Width;
			Point1i sy = y * mMaxTileSize.Height;

			if (sx < context->viewSize().Width - 1
				&& sy < context->viewSize().Height - 1)
				addTile(sx, sy);
		}
	}
	}
}

RenderTile* RenderTileMap::getNextTile()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (size_t i = 0; i < mTiles.size(); ++i)
		if (mTiles[i]->accuire())
			return mTiles[i].get();

	return nullptr;
}

bool RenderTileMap::allFinished() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (size_t i = 0; i < mTiles.size(); ++i)
		if (!mTiles[i]->isFinished() || mTiles[i]->isWorking())
			return false;
	return true;
}

void RenderTileMap::makeAllIdle()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (size_t i = 0; i < mTiles.size(); ++i)
		mTiles[i]->makeIdle();
}

RenderStatistics RenderTileMap::statistics() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	RenderStatistics s;
	for (size_t i = 0; i < mTiles.size(); ++i)
		s += mTiles[i]->statistics();
	return s;
}

double RenderTileMap::percentage() const
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	uint64 maxSamples	   = 0;
	uint64 samplesRendered = 0;
	for (size_t i = 0; i < mTiles.size(); ++i) {
		maxSamples += mTiles[i]->maxPixelSamples();
		samplesRendered += mTiles[i]->pixelSamplesRendered();
	}

	if (maxSamples == 0)
		return 100.0;
	else
		return 100 * samplesRendered / (double)maxSamples;
}

void RenderTileMap::reset()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, false);
	for (size_t i = 0; i < mTiles.size(); ++i)
		mTiles[i]->reset();
}

void RenderTileMap::optimize()
{
	PR_PROFILE_THIS;

	Mutex::scoped_lock lock(mMutex, true);
	constexpr Size1i MinTileSize = 8;
	constexpr int64 MinTimeSpent = 5 * 1000 * 1000; // 5s

	int64 fullWorkTime = 0;
	int64 validTiles   = 0;
	for (size_t i = 0; i < mTiles.size(); ++i) {
		if (mTiles[i]->lastWorkTime().count() > 0) {
			fullWorkTime += mTiles[i]->lastWorkTime().count();
			++validTiles;
		}
	}

	if (fullWorkTime == 0)
		return;

	const int64 thrWorkTime = std::max(MinTimeSpent, (int64)std::floor(fullWorkTime / (double)validTiles));
	for (auto it = mTiles.begin(); it != mTiles.end();) {
		auto ms = (*it)->lastWorkTime().count();
		if (ms < thrWorkTime || (*it)->isFinished()) {
			++it;
			continue;
		}

		auto tile = it->get();
		PR_ASSERT(!tile->isWorking(), "Did not expect a tile which is marked as working!");

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

		it = mTiles.insert(mTiles.erase(it), std::move(tiles.first));
		++it;
		it = mTiles.insert(it, std::move(tiles.second));
		++it;
	}
}
} // namespace PR

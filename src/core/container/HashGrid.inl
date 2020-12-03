// IWYU pragma: private, include "container/HashGrid.h"
namespace PR {
template <typename T, template <typename> typename PositionGetter>
HashGrid<T, PositionGetter>::HashGrid(const BoundingBox& bbox, float gridDelta, const PositionGetter<T>& getter)
	: mElements()
	, mStoredElements(0)
	, mGridDelta(gridDelta)
	, mInvGridDelta(1.0f / gridDelta)
	, mBoundingBox(bbox.expanded(0.001f))
	, mGridLength(std::max<uint32>(1, std::ceil(mBoundingBox.longestEdge() * mInvGridDelta)))
	, mPositionGetter(getter)
{
	PR_ASSERT(mGridDelta > PR_EPSILON, "Grid delta has to greater 0");
	PR_ASSERT(std::isfinite(mInvGridDelta), "Inverse of grid delta has to be valid");

	mElements.resize(mGridLength * mGridLength * mGridLength);
}

template <typename T, template <typename> typename PositionGetter>
HashGrid<T, PositionGetter>::~HashGrid()
{
}

template <typename T, template <typename> typename PositionGetter>
void HashGrid<T, PositionGetter>::reset()
{
	mStoredElements = 0;
	for (auto& m : mElements)
		m.clear();
}

template <typename T, template <typename> typename PositionGetter>
template <bool RadiusSmall, typename Function>
inline void HashGrid<T, PositionGetter>::search(const Vector3f& center, float radius2, const Function& func) const
{
	if constexpr (RadiusSmall) {
		PR_ASSERT(radius2 <= mGridDelta * mGridDelta, "Expected radius to be smaller than the configured grid delta");

		const Vector3f dp = (center - mBoundingBox.lowerBound()) * mInvGridDelta;

		const uint32 px1 = (uint32)dp(0);
		const uint32 py1 = (uint32)dp(1);
		const uint32 pz1 = (uint32)dp(2);

		// An underflow may occur, but this is ok
		const uint32 px2 = (uint32)(px1 + (dp(0) - px1 > 0.5f ? 1 : -1));
		const uint32 py2 = (uint32)(py1 + (dp(1) - py1 > 0.5f ? 1 : -1));
		const uint32 pz2 = (uint32)(pz1 + (dp(2) - pz1 > 0.5f ? 1 : -1));

		for (int i = 0; i < 8; i++) {
			const uint32 nx = (i & 1) != 0 ? px2 : px1;
			const uint32 ny = (i & 2) != 0 ? py2 : py1;
			const uint32 nz = (i & 4) != 0 ? pz2 : pz1;

			if (PR_UNLIKELY(nx >= mGridLength || ny >= mGridLength || nz >= mGridLength))
				continue;

			const KeyCoord key = { nx, ny, nz };
			for (const auto& el : mElements[toIndex(key)])
				func(el);
		}

	} else {
		const KeyCoord centerCoord = toCoords(center);
		const int32 rad2		   = (int32)std::ceil(radius2 * mInvGridDelta * mInvGridDelta);

		for (int x = -rad2; x <= rad2; ++x) {
			for (int y = -rad2; y <= rad2; ++y) {
				if (x * x + y * y > rad2)
					continue;

				for (int z = -rad2; z <= rad2; ++z) {
					if (x * x + y * y + z * z > rad2)
						continue;

					const int nx = centerCoord.X + x;
					const int ny = centerCoord.Y + y;
					const int nz = centerCoord.Z + z;
					if (PR_UNLIKELY(nx < 0 || nx >= (int)mGridLength || ny < 0 || ny >= (int)mGridLength || nz < 0 || nz >= (int)mGridLength))
						continue;

					const KeyCoord key = { (uint32)nx, (uint32)ny, (uint32)nz };
					for (const auto& el : mElements[toIndex(key)])
						func(el);
				}
			}
		}
	}
}

template <typename T, template <typename> typename PositionGetter>
void HashGrid<T, PositionGetter>::store(const T& el)
{
	const Vector3f pos = mPositionGetter(el);
	if (!mBoundingBox.contains(pos))
		return;

	storeUnsafe(el);
}

template <typename T, template <typename> typename PositionGetter>
void HashGrid<T, PositionGetter>::storeUnsafe(const T& el)
{
	mStoredElements++;
	const Vector3f pos = mPositionGetter(el);
	const auto key	   = toCoords(pos);

	mElements[toIndex(key)].push_back(el);
}

template <typename T, template <typename> typename PositionGetter>
typename HashGrid<T, PositionGetter>::KeyCoord HashGrid<T, PositionGetter>::toCoords(const Vector3f& p) const
{
	const Vector3f dp = (p - mBoundingBox.lowerBound()) * mInvGridDelta;
	return {
		static_cast<uint32>(dp(0)),
		static_cast<uint32>(dp(1)),
		static_cast<uint32>(dp(2))
	};
}

template <typename T, template <typename> typename PositionGetter>
bool HashGrid<T, PositionGetter>::KeyCoord::operator==(const KeyCoord& other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

template <typename T, template <typename> typename PositionGetter>
inline size_t HashGrid<T, PositionGetter>::toIndex(const KeyCoord& coords) const
{
	return xyz_2_morton(coords.X, coords.Y, coords.Z) % (mElements.size() - 1);
}

} // namespace PR

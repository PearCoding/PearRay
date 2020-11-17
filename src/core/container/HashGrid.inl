// IWYU pragma: private, include "container/HashGrid.h"
namespace PR {
template <typename T, template <typename> typename PositionGetter>
HashGrid<T, PositionGetter>::HashGrid(const BoundingBox& bbox, float gridDelta, const PositionGetter<T>& getter)
	: mElements()
	, mStoredElements(0)
	, mGridDelta(gridDelta)
	, mInvGridDelta(1.0f / gridDelta)
	, mBoundingBox(bbox.expanded(0.001f))
	, mGridX(std::max<uint32>(1, std::ceil(mBoundingBox.edge(0) * mInvGridDelta)))
	, mGridY(std::max<uint32>(1, std::ceil(mBoundingBox.edge(1) * mInvGridDelta)))
	, mGridZ(std::max<uint32>(1, std::ceil(mBoundingBox.edge(2) * mInvGridDelta)))
	, mPositionGetter(getter)
{
	PR_ASSERT(mGridDelta > PR_EPSILON, "Grid delta has to greater 0");
	PR_ASSERT(std::isfinite(mInvGridDelta), "Inverse of grid delta has to be valid");

	mElements.resize(mGridX * mGridY * mGridZ);
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
template <typename Function>
inline void HashGrid<T, PositionGetter>::search(const Vector3f& center, float radius, const Function& func) const
{
	KeyCoord centerCoord = toCoords(center);
	const int32 rad		 = std::max<int32>(0, (int32)std::ceil(radius * mInvGridDelta)) + 1;
	const int32 rad2	 = rad * rad;

	for (int x = -rad; x <= rad; ++x) {
		for (int y = -rad; y <= rad; ++y) {
			if (x * x + y * y > rad2)
				continue;

			for (int z = -rad; z <= rad; ++z) {
				if (x * x + y * y + z * z > rad2)
					continue;

				const int nx = centerCoord.X + x;
				const int ny = centerCoord.Y + y;
				const int nz = centerCoord.Z + z;
				if (nx < 0 || nx >= (int)mGridX || ny < 0 || ny >= (int)mGridY || nz < 0 || nz >= (int)mGridZ)
					continue;

				const KeyCoord key = { (uint32)nx, (uint32)ny, (uint32)nz };
				for (const auto& el : mElements[toIndex(key)])
					func(el);
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
		static_cast<uint32>(std::floor(dp(0))),
		static_cast<uint32>(std::floor(dp(1))),
		static_cast<uint32>(std::floor(dp(2)))
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
	return coords.X + coords.Y * mGridX + coords.Z * mGridX * mGridY;
}

} // namespace PR

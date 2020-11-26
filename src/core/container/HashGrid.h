#pragma once

#include "geometry/BoundingBox.h"
#include "math/Hash.h"

#include "PositionGetter.h"

#include <atomic>
#include <tbb/concurrent_vector.h>
#include <vector>

namespace PR {

// Spatial Hashmap
template <typename T, template <typename> typename PositionGetter = position_getter>
class HashGrid {
	PR_CLASS_NON_COPYABLE(HashGrid);

public:
	template <typename U = T>
	inline HashGrid(const BoundingBox& bbox, float gridDelta,
					typename std::enable_if<std::is_default_constructible<PositionGetter<U>>::value>::type* = 0)
		: HashGrid(bbox, gridDelta, PositionGetter<U>())
	{
	}

	inline HashGrid(const BoundingBox& bbox, float gridDelta, const PositionGetter<T>& getter);
	inline virtual ~HashGrid();

	inline void reset();

	inline bool isEmpty() const { return mStoredElements == 0; }
	inline uint64 storedElements() const { return mStoredElements; }

	template <typename Function>
	inline void search(const Vector3f& center, float radius, const Function& func) const;

	inline void store(const T& point);
	inline void storeUnsafe(const T& point); // Do not check for the boundary

private:
	struct KeyCoord {
		uint32 X, Y, Z;

		inline bool operator==(const KeyCoord& other) const;
	};

	inline KeyCoord toCoords(const Vector3f& pos) const;
	inline size_t toIndex(const KeyCoord& coords) const;

	using Map = std::vector<tbb::concurrent_vector<T>>;
	Map mElements;

	std::atomic<uint64> mStoredElements;
	const float mGridDelta;
	const float mInvGridDelta;
	const BoundingBox mBoundingBox;
	const uint32 mGridX;
	const uint32 mGridY;
	const uint32 mGridZ;
	const PositionGetter<T> mPositionGetter;
};
} // namespace PR

#include "HashGrid.inl"

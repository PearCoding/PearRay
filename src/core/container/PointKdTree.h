#pragma once

#include "geometry/BoundingBox.h"

#include "PositionGetter.h"

namespace PR {
// Based on the implementation in the book:
// Realistic Image Synthesis Using Photon Mapping (2nd Edition: 2001)
// from Henrik Wann Jensen
template <class T, template <typename> typename PositionGetter = position_getter>
class PointKdTree {
	PR_CLASS_NON_COPYABLE(PointKdTree);

public:
	template <typename U = T>
	PointKdTree(size_t max_points,
				typename std::enable_if<std::is_default_constructible<PositionGetter<U>>::value>::type* = 0)
		: PointKdTree(max_points, PositionGetter<U>())
	{
	}
	inline PointKdTree(size_t max_points, const PositionGetter<T>& getter);
	virtual ~PointKdTree();

	inline void reset();

	inline bool isFull() const { return mStoredElements >= mMaxElements; }
	inline bool isEmpty() const { return mStoredElements == 0; }
	inline uint64 storedElements() const { return mStoredElements; }

	template <typename Function>
	inline void search(const Vector3f& center, float radius, const Function& func) const;

	inline void store(const T& point);

	inline void balanceTree(); // Balance the KD-tree before using

private:
	struct ElementWrapper {
		T Element;
		uint8 Axis = 0;
	};

	// KD-tree utils
	inline void balanceSegment(ElementWrapper** balance, ElementWrapper** original,
							   uint64 index, uint64 start, uint64 end);
	inline void medianSplit(ElementWrapper** elements, uint64 start, uint64 end, uint64 median, int axis);

	template <typename Function>
	inline void internalSearch(const Vector3f& center, float radius2, size_t index, const Function& func) const;

	std::vector<ElementWrapper> mElements;
	size_t mStoredElements;
	size_t mHalfStoredElements;
	const size_t mMaxElements;
	bool mBalanced;

	BoundingBox mBox;

	const PositionGetter<T> mPositionGetter;
};
} // namespace PR

#include "PointKdTree.inl"
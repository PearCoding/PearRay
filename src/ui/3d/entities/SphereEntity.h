#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI SphereEntity : public GraphicEntity {
public:
	SphereEntity(float radius = 1.0f);
	virtual ~SphereEntity();

	void setRadius(float h);
	inline float radius() const { return mRadius; }

	void setStackCount(uint32 count);
	inline uint32 stackCount() const { return mStackCount; }

	void setSliceCount(uint32 count);
	inline uint32 sliceCount() const { return mSliceCount; }

private:
	void setupGeometry();

	float mRadius;
	uint32 mStackCount;
	uint32 mSliceCount;
};
} // namespace UI
} // namespace PR
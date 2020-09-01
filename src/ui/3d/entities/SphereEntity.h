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

	void setStackCount(unsigned int count);
	inline unsigned int stackCount() const { return mStackCount; }

	void setSliceCount(unsigned int count);
	inline unsigned int sliceCount() const { return mSliceCount; }

private:
	void setupGeometry();

	float mRadius;
	unsigned int mStackCount;
	unsigned int mSliceCount;
};
} // namespace UI
} // namespace PR
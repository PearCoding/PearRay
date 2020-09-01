#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI CylinderEntity : public GraphicEntity {
public:
	CylinderEntity(float height = 1.0f, float topRadius = 1.0f, float bottomRadius = 1.0f);
	virtual ~CylinderEntity();

	void setHeight(float h);
	inline float height() const { return mHeight; }

	void setBottomRadius(float h);
	inline float bottomRadius() const { return mBottomRadius; }

	void setTopRadius(float h);
	inline float topRadius() const { return mTopRadius; }

	void setSectionCount(unsigned int count);
	inline unsigned int sectionCount() const { return mSectionCount; }

private:
	void setupGeometry();

	float mHeight;
	float mTopRadius;
	float mBottomRadius;
	unsigned int mSectionCount;
};
} // namespace UI
} // namespace PR
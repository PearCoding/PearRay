#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI ConeEntity : public GraphicEntity {
public:
	ConeEntity(float height = 1.0f, float radius = 1.0f);
	virtual ~ConeEntity();

	void setHeight(float h);
	inline float height() const { return mHeight; }

	void setRadius(float h);
	inline float radius() const { return mRadius; }

	void setSectionCount(unsigned int count);
	inline unsigned int sectionCount() const { return mSectionCount; }

private:
	void setupGeometry();

	float mHeight;
	float mRadius;
	unsigned int mSectionCount;
};
} // namespace UI
} // namespace PR
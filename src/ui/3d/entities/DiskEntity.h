#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI DiskEntity : public GraphicEntity {
public:
	DiskEntity(float radius = 1.0f);
	virtual ~DiskEntity();

	void setRadius(float h);
	inline float radius() const { return mRadius; }

	Vector3f normal() const;

	void setSectionCount(unsigned int count);
	inline unsigned int sectionCount() const { return mSectionCount; }

private:
	void setupGeometry();

	float mRadius;
	unsigned int mSectionCount;
};
} // namespace UI
} // namespace PR
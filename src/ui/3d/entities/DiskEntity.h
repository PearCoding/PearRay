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

	void setSectionCount(uint32 count);
	inline uint32 sectionCount() const { return mSectionCount; }

private:
	void setupGeometry();

	float mRadius;
	uint32 mSectionCount;
};
} // namespace UI
} // namespace PR
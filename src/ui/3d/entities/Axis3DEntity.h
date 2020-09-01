#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI Axis3DEntity : public GraphicEntity {
public:
	Axis3DEntity(float edgeLength = 1.0f);
	virtual ~Axis3DEntity();

	void setEdgeLength(float f);
	inline float edgeLength() const { return mEdgeLength; }

private:
	void setupBuffer();

	float mEdgeLength;
};
} // namespace UI
} // namespace PR
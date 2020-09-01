#pragma once

#include "GraphicEntity.h"
#include "geometry/BoundingBox.h"

namespace PR {
namespace UI {
class PR_LIB_UI AABBEntity : public GraphicEntity {
public:
	AABBEntity(const BoundingBox& aabb = BoundingBox());
	virtual ~AABBEntity();

	void setAABB(const BoundingBox& o);
	inline const BoundingBox& aabb() const { return mAABB; }

private:
	void setupBuffer();

	BoundingBox mAABB;
};
} // namespace UI
} // namespace PR
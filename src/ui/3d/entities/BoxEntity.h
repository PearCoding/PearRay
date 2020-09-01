#pragma once

#include "GraphicEntity.h"
#include "geometry/BoundingBox.h"

namespace PR {
namespace UI {
// Same as AABBEntity, but filled
class PR_LIB_UI BoxEntity : public GraphicEntity {
public:
	BoxEntity(const BoundingBox& aabb = BoundingBox());
	virtual ~BoxEntity();

	void setAABB(const BoundingBox& o);
	inline const BoundingBox& aabb() const { return mAABB; }

private:
	void setupBuffer();

	BoundingBox mAABB;
};
} // namespace UI
}
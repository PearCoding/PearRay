#include "RenderEntity.h"

#include "material/Material.h"

#include "performance/Performance.h"

namespace PR
{
	RenderEntity::RenderEntity(uint32 id, const std::string& name) :
		Entity(id, name)
	{
	}

	RenderEntity::~RenderEntity()
	{
	}

	BoundingBox RenderEntity::calcWorldBoundingBox() const
	{
		PR_GUARD_PROFILE();
		
		const BoundingBox bx = localBoundingBox();
		const PM::mat mat = matrix();

		const PM::vec3 v1 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.upperBound()), PM::pm_GetY(bx.upperBound()), PM::pm_GetZ(bx.upperBound()), 1));
		const PM::vec3 v2 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.lowerBound()), PM::pm_GetY(bx.upperBound()), PM::pm_GetZ(bx.upperBound()), 1));
		const PM::vec3 v3 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.lowerBound()), PM::pm_GetY(bx.lowerBound()), PM::pm_GetZ(bx.upperBound()), 1));
		const PM::vec3 v4 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.lowerBound()), PM::pm_GetY(bx.upperBound()), PM::pm_GetZ(bx.lowerBound()), 1));
		const PM::vec3 v5 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.lowerBound()), PM::pm_GetY(bx.lowerBound()), PM::pm_GetZ(bx.lowerBound()), 1));
		const PM::vec3 v6 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.upperBound()), PM::pm_GetY(bx.lowerBound()), PM::pm_GetZ(bx.upperBound()), 1));
		const PM::vec3 v7 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.upperBound()), PM::pm_GetY(bx.lowerBound()), PM::pm_GetZ(bx.lowerBound()), 1));
		const PM::vec3 v8 = PM::pm_Transform(mat,
			PM::pm_Set(PM::pm_GetX(bx.upperBound()), PM::pm_GetY(bx.upperBound()), PM::pm_GetZ(bx.lowerBound()), 1));

		BoundingBox w(v1, v2);
		w.put(v3); w.put(v4); w.put(v5);
		w.put(v6); w.put(v7); w.put(v8);
		return w;
	}

	void RenderEntity::onFreeze()
	{
		Entity::onFreeze();

		mWorldBoundingBox_Cache = calcWorldBoundingBox();
	}
}
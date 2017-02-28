#include "MeshEntity.h"

#include "material/Material.h"
#include "geometry/TriMesh.h"
#include "geometry/Face.h"
#include "geometry/Triangle.h"
#include "shader/FaceSample.h"

#include "math/Projection.h"

#include "Logger.h"

#include "performance/Performance.h"

namespace PR
{
	MeshEntity::MeshEntity(uint32 id, const std::string& name) :
		RenderEntity(id, name), mMesh(nullptr)
	{
	}

	MeshEntity::~MeshEntity()
	{
	}

	std::string MeshEntity::type() const
	{
		return "mesh";
	}

	bool MeshEntity::isLight() const
	{
		for(auto ptr : mMaterials)
		{
			if(ptr->isLight())
				return true;
		}

		return false;
	}

	float MeshEntity::surfaceArea(Material* m) const
	{
		if(isFrozen() && (m == nullptr))
			return mSurfaceArea_Cache;
		else
		{
			if(m)
			{
				uint32 slot;
				for(slot = 0; slot < mMaterials.size(); ++slot)
				{
					if(mMaterials[slot].get() == m)
						break;
				}

				if(slot >= mMaterials.size())
					return mMesh->surfaceArea(flags() & EF_LocalArea ? PM::pm_Identity() : matrix());
				else
					return mMesh->surfaceArea(slot, flags() & EF_LocalArea ? PM::pm_Identity() : matrix());
			}
			else
			{
				return mMesh->surfaceArea(flags() & EF_LocalArea ? PM::pm_Identity() : matrix());
			}
		}
	}

	void MeshEntity::setMesh(const std::shared_ptr<TriMesh>& mesh)
	{
		mMesh = mesh;
	}

	const std::shared_ptr<TriMesh>& MeshEntity::mesh() const
	{
		return mMesh;
	}

	void MeshEntity::reserveMaterialSlots(size_t count)
	{
		mMaterials.resize(count);
	}

	void MeshEntity::setMaterial(uint32 slot, const std::shared_ptr<Material>& m)
	{
		PR_ASSERT(slot < mMaterials.size(), "material slots not correctly reserved!");
		mMaterials[slot] = m;
	}

	std::shared_ptr<Material> MeshEntity::material(uint32 slot) const
	{
		if(slot >= mMaterials.size())
			return mMaterials.front();
		else
			return mMaterials[slot];
	}

	bool MeshEntity::isCollidable() const
	{
		if(!mMesh)
			return false;
		
		for(auto ptr : mMaterials)
		{
			if(ptr->canBeShaded())
				return true;
		}

		return false;
	}

	float MeshEntity::collisionCost() const
	{
		return mMesh->collisionCost();
	}

	BoundingBox MeshEntity::localBoundingBox() const
	{
		PR_ASSERT(mMesh, "mesh has to be initialized before accessing");
		return mMesh->boundingBox();
	}

	bool MeshEntity::checkCollision(const Ray& ray, FaceSample& collisionPoint) const
	{
		PR_GUARD_PROFILE();

		// Local space
		Ray local = ray;
		local.setStartPosition(PM::pm_Multiply(invMatrix(), ray.startPosition()));
		local.setDirection(PM::pm_Normalize3D(PM::pm_Multiply(invDirectionMatrix(), ray.direction())));

		Face* f = mMesh->checkCollision(local, collisionPoint);
		if (f)
		{
			collisionPoint.P = PM::pm_Multiply(matrix(), collisionPoint.P);
			collisionPoint.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), collisionPoint.Ng));
			Projection::tangent_frame(collisionPoint.Ng, collisionPoint.Nx, collisionPoint.Ny);

			collisionPoint.Material = material(f->MaterialSlot).get();

			return true;
		}
		else
		{
			return false;
		}
	}

	FaceSample MeshEntity::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		PR_GUARD_PROFILE();

		uint32 material_slot;
		FaceSample point = mMesh->getRandomFacePoint(sampler, sample, material_slot, pdf);
		point.Ng = PM::pm_Normalize3D(PM::pm_Multiply(directionMatrix(), point.Ng));
		point.P = PM::pm_Multiply(matrix(), point.P);
		Projection::tangent_frame(point.Ng, point.Nx, point.Ny);

		point.Material = material(material_slot).get();

		return point;
	}

	void MeshEntity::onFreeze()
	{
		RenderEntity::onFreeze();

		mSurfaceArea_Cache = mMesh->surfaceArea(flags() & EF_LocalArea ? PM::pm_Identity() : matrix());

		// Check up
		if(mSurfaceArea_Cache <= PM_EPSILON)
			PR_LOGGER.logf(L_Warning, M_Entity, "Mesh entity %s has zero surface area!", name().c_str());
	}

	void MeshEntity::setup(RenderContext* context)
	{
		RenderEntity::setup(context);

		for(auto ptr : mMaterials)
			ptr->setup(context);
	}
	
	std::string MeshEntity::dumpInformation() const
	{
		std::stringstream stream;
		stream << RenderEntity::dumpInformation()
			<< "    <MeshEntity>: " << std::endl
			<< "      Material Count: " << mMaterials.size() << std::endl;

		return stream.str();
	}
}

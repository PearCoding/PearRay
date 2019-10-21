#include "geometry/Plane.h"
#include "Environment.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Tangent.h"
#include "registry/Registry.h"

namespace PR {

class PlaneEntity : public IEntity {
public:
	ENTITY_CLASS

	PlaneEntity(uint32 id, const std::string& name,
				const Vector3f& xAxis, const Vector3f& yAxis,
				int32 matID, int32 lightID)
		: IEntity(id, name)
		, mPlane(Vector3f(0, 0, 0), xAxis, yAxis)
		, mMaterialID(matID)
		, mLightID(lightID)
		, mPDF_Cache(0.0f)
	{
	}
	virtual ~PlaneEntity() {}

	std::string type() const override
	{
		return "plane";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 id) const override
	{
		if (id == 0 || mMaterialID < 0 || id == mMaterialID)
			return mPlane.surfaceArea();
		else
			return 0;
	}

	bool isCollidable() const override
	{
		return mPlane.isValid();
	}

	float collisionCost() const override
	{
		return 1;
	}

	BoundingBox localBoundingBox() const override
	{
		return mPlane.toBoundingBox();
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());

		mPlane.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= simdpp::make_uint(id());
		out.FaceID		= simdpp::make_uint(0);
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		auto in_local = in.transform(invTransform().matrix(), invDirectionMatrix());

		mPlane.intersects(in_local, out);

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.EntityID	= id();
		out.FaceID		= 0;
		out.MaterialID  = mMaterialID;
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& faceID, float& pdf) const override
	{
		pdf	= mPDF_Cache;
		faceID = 0;
		return rnd;
	}

	void provideGeometryPoint(uint32, float u, float v,
							  GeometryPoint& pt) const override
	{
		pt.P  = transform() * mPlane.surfacePoint(u, v);
		pt.N  = directionMatrix() * mPlane.normal();
		pt.Nx = directionMatrix() * mPlane.xAxis().normalized();
		pt.Ny = directionMatrix() * mPlane.yAxis().normalized();

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UVW		  = Vector3f(u, v, 0);
		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

	inline void centerOn()
	{
		mPlane.setPosition(-0.5f * mPlane.xAxis() - 0.5f * mPlane.yAxis());
	}

protected:
	void onFreeze(RenderContext* context) override
	{
		IEntity::onFreeze(context);

		const float area = surfaceArea(0);
		mPDF_Cache		 = (area > PR_EPSILON ? 1.0f / area : 0);
	}

private:
	Plane mPlane;
	int32 mMaterialID;
	int32 mLightID;

	float mPDF_Cache;
};

class PlaneEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const auto& reg = env.registry();

		std::string name = reg.getForObject<std::string>(RG_ENTITY, uuid, "name",
														 "__unnamed__");
		Vector3f xAxis   = reg.getForObject<Vector3f>(RG_ENTITY, uuid, "x_axis",
													  Vector3f(1, 0, 0));
		Vector3f yAxis   = reg.getForObject<Vector3f>(RG_ENTITY, uuid, "y_axis",
													  Vector3f(0, 1, 0));
		float width		 = reg.getForObject<float>(RG_ENTITY, uuid, "width", 1);
		float height	 = reg.getForObject<float>(RG_ENTITY, uuid, "height", 1);

		std::string emsName = reg.getForObject<std::string>(RG_ENTITY, uuid, "emission", "");
		std::string matName = reg.getForObject<std::string>(RG_ENTITY, uuid, "material", "");

		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = env.getMaterial(matName);
		if (mat)
			matID = mat->id();

		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = ems->id();

		auto obj = std::make_shared<PlaneEntity>(id, name, width*xAxis, height*yAxis, matID, emsID);
		if (reg.getForObject<bool>(RG_ENTITY, uuid, "centering", false))
			obj->centerOn();

		return obj;
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "plane" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PlaneEntityFactory, "ent_plane", PR_PLUGIN_VERSION)
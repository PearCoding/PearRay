#include "curve/Curve.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityFactory.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"

#include <boost/filesystem.hpp>

namespace PR {

template <typename T>
class CurveEntity : public IEntity {
public:
	ENTITY_CLASS

	CurveEntity(uint32 id, const std::string& name,
				const T& curve,
				Vector2f width,
				int32 materialID, int32 lightID)
		: IEntity(id, name)
		, mLightID(lightID)
		, mMaterialID(materialID)
		, mCurve(curve)
		, mWidth(width)
	{
	}

	virtual ~CurveEntity() {}

	std::string type() const override
	{
		return "curve";
	}

	bool isLight() const override
	{
		return mLightID >= 0;
	}

	float surfaceArea(uint32 /*id*/) const override
	{
		// TODO
		return 1.0f;
	}

	bool isCollidable() const override
	{
		return mMaterialID >= 0 && mCurve.isValid();
	}

	float collisionCost() const override
	{
		return 10;
	}

	BoundingBox localBoundingBox() const override
	{
		BoundingBox box;
		for (size_t i = 0; i <= mCurve.degree(); ++i)
			box.combine(mCurve.point(i));

		box.expand(std::max(mWidth(0), mWidth(1)) * 0.5f);
		return box;
	}

	void checkCollision(const RayPackage& in, CollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		// TODO

		out.HitDistance = in_local.distanceTransformed(out.HitDistance,
													   transform().matrix(), in);
		out.FaceID		= simdpp::make_uint(0);
		out.EntityID	= simdpp::make_uint(id());
		out.MaterialID  = simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, SingleCollisionOutput& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		Vector3f dx, dy;
		Tangent::frame(in.Direction, dx, dy);

		// Project to 2d plane
		Eigen::Matrix4f mat;
		mat.block<3, 1>(0, 0) = dx.cross(in.Direction);
		mat.block<3, 1>(0, 1) = in.Direction.cross(mat.block<3, 1>(0, 0));
		mat.block<3, 1>(0, 2) = in_local.Direction;
		mat.block<3, 1>(0, 3) = in_local.Origin;
		mat.block<1, 4>(3, 0) = Vector4f(0, 0, 0, 1);
		Eigen::Matrix4f proj  = mat.inverse();

		T projectedCurve = mCurve;
		for (size_t i = 0; i <= projectedCurve.degree(); ++i) {
			Vector4f projP			   = proj * Vector4f(mCurve.point(i)(0), mCurve.point(i)(1), mCurve.point(i)(2), 1.0f);
			projectedCurve.points()[i] = Vector3f(projP(0), projP(1), projP(2)) / projP(3);
		}

		// Calculate max depth
		float l0 = 0;
		for (size_t i = 0; i < projectedCurve.degree() - 1; ++i) {
			l0 = std::max(l0,
						  std::max(std::max(std::abs(projectedCurve.point(i)(0) - 2 * projectedCurve.point(i + 1)(0) + projectedCurve.point(i + 2)(0)),
											std::abs(projectedCurve.point(i)(1) - 2 * projectedCurve.point(i + 1)(1) + projectedCurve.point(i + 2)(1))),
								   std::abs(projectedCurve.point(i)(2) - 2 * projectedCurve.point(i + 1)(2) + projectedCurve.point(i + 2)(2))));
		}

		float eps		= std::max(mWidth(0), mWidth(1)) / 20.f;
		float fr0		= std::log(1.41421356237f * 12.0f * l0 / (8.0f * eps)) / std::log(4);
		size_t maxDepth = static_cast<size_t>(std::max(0, std::min(10, (int)std::round(fr0))));

		if (!recursiveCheck(in_local, out, projectedCurve, 0, 1, maxDepth))
			out.HitDistance = std::numeric_limits<float>::infinity();
		else {
			out.HitDistance = in_local.distanceTransformed(out.HitDistance,
														   transform().matrix(), in);
			out.FaceID		= 0;
			out.EntityID	= id();
			out.MaterialID  = mMaterialID;
		}
	}

	Vector2f pickRandomPoint(const Vector2f& rnd, uint32& /*faceID*/, float& pdf) const override
	{
		PR_PROFILE_THIS;
		pdf = 1 / surfaceArea(0);
		return rnd;
	}

	void provideGeometryPoint(uint32 /*faceID*/, float u, float v,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		float width = mWidth(0) * (1 - u) + mWidth(1) * u;
		// TODO: Require ray information to make is view aligned
		pt.P  = mCurve.startPoint(); //mCurve.eval(u) /*+ mCurve.evalDerivative(u) * (1 - 2 * v) * width / 2*/;
		pt.N  = Vector3f(0, 0, 1);
		pt.Nx = Vector3f(1, 0, 0);
		pt.Ny = Vector3f(0, 1, 0);

		// Global
		pt.P  = transform() * pt.P;
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.MaterialID = mMaterialID;
		pt.EmissionID = mLightID;
		pt.DisplaceID = 0;
	}

private:
	bool recursiveCheck(const Ray& in, SingleCollisionOutput& out,
						const T& curve,
						float uMin, float uMax, size_t depth) const
	{
		// Culling
		BoundingBox box;
		for (size_t i = 0; i < curve.degree() + 1; ++i)
			box.combine(curve.point(i));

		float width = std::max(mWidth(0) * (1 - uMin) + mWidth(1) * uMin,
							   mWidth(0) * (1 - uMax) + mWidth(1) * uMax);
		box.expand(width / 2);

		box.intersects(Ray(Vector3f(0, 0, -1), Vector3f(0, 0, 1)), out);
		if (out.HitDistance == std::numeric_limits<float>::infinity())
			return false;

		if (depth > 0) {
			float mid = (uMin + uMax) / 2;
			T firstHalf, secondHalf;
			curve.subdivide(firstHalf, secondHalf);
			return recursiveCheck(in, out, firstHalf, uMin, mid, depth - 1)
				   || recursiveCheck(in, out, secondHalf, mid, uMax, depth - 1);
		} else {
			// Curve segments
			Vector3f s0 = curve.startPoint();
			//Vector3f s1 = curve.point(1);
			Vector3f e0 = curve.endPoint();
			//Vector3f e1 = curve.point(curve.degree() - 1);

			/*float edge0 = s0(0) * (s0(0) - s1(0)) - (s0(1) - s1(1)) * s0(1);
			float edge1 = e0(0) * (e0(0) - e1(0)) - (e0(1) - e1(1)) * e0(1);
			if (edge0 < 0 || edge1 < 0)
				return false;*/

			// Minimum distance
			Vector3f dir = e0 - s0;
			float denom  = dir.block<2, 1>(0, 0).squaredNorm();
			if (std::abs(denom) < PR_EPSILON)
				return false;

			float w = -dir.block<2, 1>(0, 0).dot(s0.block<2, 1>(0, 0)) / denom;

			// U coordinate
			float u		   = std::max(uMin, std::min(uMax, uMin * (1 - w) + uMax * w));
			float hitwidth = mWidth(0) * (1 - u) + mWidth(1) * u;

			// Intersection point
			Vector3f p  = curve.eval(std::min(1.0f, std::max(0.0f, w)));
			float dist2 = p.block<2, 1>(0, 0).squaredNorm();
			if (dist2 > hitwidth * hitwidth * 0.25f)
				return false;

			if (p(2) < 0)
				return false;

			// V coordinate
			Vector3f dp = curve.evalDerivative(std::min(1.0f, std::max(0.0f, w)));
			float dist  = std::sqrt(dist2);
			float edge  = p(0) * dp(1) - dp(0) * p(1);
			float v		= (edge > 0)
						  ? 0.5f + dist / hitwidth
						  : 0.5f - dist / hitwidth;

			out.UV[0]		= u;
			out.UV[1]		= v;
			out.HitDistance = p(2);

			return true;
		}
	}

	int32 mLightID;
	int32 mMaterialID;
	T mCurve;
	Vector2f mWidth;
};

class CurveEntityFactory : public IEntityFactory {
public:
	std::shared_ptr<IEntity> create(uint32 id, uint32 uuid, const Environment& env)
	{
		const Registry& reg = env.registry();
		std::string name	= reg.getForObject<std::string>(RG_ENTITY, uuid, "name", "__unnamed__");
		uint32 degree		= (uint32)std::max(
			  1,
			  reg.getForObject<int>(RG_ENTITY, uuid, "degree", 3));

		std::vector<float> points = reg.getForObject<std::vector<float>>(
			RG_ENTITY, uuid, "points", std::vector<float>());
		if (points.size() != (degree + 1) * 3) {
			PR_LOG(L_ERROR) << "Curve " << name << " is invalid. Expected " << (degree + 1) * 3 << " entries in points" << std::endl;
			return nullptr;
		}

		Vector2f width = reg.getForObject<Vector2f>(RG_ENTITY, uuid, "width", Vector2f(1, 1));

		std::string matName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "material", "");
		int32 matID					   = -1;
		std::shared_ptr<IMaterial> mat = env.getMaterial(matName);
		if (mat)
			matID = static_cast<int32>(mat->id());

		std::string emsName = reg.getForObject<std::string>(
			RG_ENTITY, uuid, "emission", "");
		int32 emsID					   = -1;
		std::shared_ptr<IEmission> ems = env.getEmission(emsName);
		if (ems)
			emsID = static_cast<int32>(ems->id());

		switch (degree) {
		case 2: {
			QuadraticCurve3 curve;
			for (size_t i = 0; i <= curve.degree(); ++i)
				curve.points()[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);
			return std::make_shared<CurveEntity<QuadraticCurve3>>(id, name,
																  curve, width,
																  matID, emsID);
		}
		case 3: {
			CubicCurve3 curve;
			for (size_t i = 0; i <= curve.degree(); ++i)
				curve.points()[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);
			return std::make_shared<CurveEntity<CubicCurve3>>(id, name,
															  curve, width,
															  matID, emsID);
		}
		default: {
			Curve3::PointList list(degree + 1);
			for (size_t i = 0; i < list.size(); ++i)
				list[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);

			return std::make_shared<CurveEntity<Curve3>>(id, name,
														 Curve3(list), width,
														 matID, emsID);
		}
		}
	}

	const std::vector<std::string>& getNames() const
	{
		static std::vector<std::string> names({ "curve", "bezier" });
		return names;
	}

	bool init()
	{
		return true;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CurveEntityFactory, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
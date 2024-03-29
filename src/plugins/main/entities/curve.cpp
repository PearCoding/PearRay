#include "curve/Curve.h"
#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "SceneLoadContext.h"
#include "emission/IEmission.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/CollisionData.h"
#include "material/IMaterial.h"
#include "math/Projection.h"
#include "math/Transform.h"

#include <filesystem>

// TODO: Change to new geometry representation

namespace PR {
template <typename T>
class CurveEntity : public IEntity {
public:
	ENTITY_CLASS

	CurveEntity(uint32 id, const std::string& name, const Transformf& transform,
				const T& curve,
				Vector2f width,
				uint32 materialID, uint32 lightID)
		: IEntity(id, lightID, name, transform)
		, mMaterialID(materialID)
		, mCurve(curve)
		, mWidth(width)
		, mApproxSurfaceArea(0.0f)
	{
	}

	virtual ~CurveEntity() {}

	std::string type() const override
	{
		return "curve";
	}

	float localSurfaceArea(uint32 /*id*/) const override
	{
		return mApproxSurfaceArea;
	}

	bool isCollidable() const override
	{
		return mMaterialID != PR_INVALID_ID && mCurve.isValid();
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

		out.HitDistance = in_local.transformDistance(out.HitDistance,
													 transform().linear());
		out.FaceID		= simdpp::make_uint(0);
		out.EntityID	= simdpp::make_uint(id());
		out.MaterialID	= simdpp::make_uint(mMaterialID);
	}

	void checkCollision(const Ray& in, HitPoint& out) const override
	{
		PR_PROFILE_THIS;

		auto in_local = in.transformAffine(invTransform().matrix(), invTransform().linear());

		Vector3f dx, dy;
		Tangent::frame(in_local.Direction, dx, dy);

		// Project to 2d plane
		Eigen::Matrix3f proj = PR::Transform::orthogonalInverse(PR::Transform::orthogonalMatrix(dy, dx, in_local.Direction));

		T projectedCurve = mCurve;
		for (size_t i = 0; i <= projectedCurve.degree(); ++i)
			projectedCurve.points()[i] = proj * (mCurve.point(i) - in_local.Origin);

		// Calculate max depth
		float l0 = 0;
		for (size_t i = 0; i < projectedCurve.degree() - 1; ++i) {
			l0 = std::max(l0,
						  std::max(std::max(std::abs(projectedCurve.point(i)(0) - 2 * projectedCurve.point(i + 1)(0) + projectedCurve.point(i + 2)(0)),
											std::abs(projectedCurve.point(i)(1) - 2 * projectedCurve.point(i + 1)(1) + projectedCurve.point(i + 2)(1))),
								   std::abs(projectedCurve.point(i)(2) - 2 * projectedCurve.point(i + 1)(2) + projectedCurve.point(i + 2)(2))));
		}

		float eps		= std::max(mWidth(0), mWidth(1)) / 20.f;
		float fr0		= static_cast<float>(std::log(1.41421356237f * 12.0f * l0 / (8.0f * eps)) / std::log(4));
		size_t maxDepth = static_cast<size_t>(std::max(0, std::min(10, (int)std::round(fr0))));

		out.Successful = recursiveCheck(in_local, out, projectedCurve, 0, 1, maxDepth);
		if (out.Successful) {
			out.HitDistance = in_local.transformDistance(out.HitDistance,
														 transform().linear());
			out.FaceID		= 0;
			out.EntityID	= id();
			out.MaterialID	= mMaterialID;
		}
	}

	Vector3f pickRandomParameterPoint(const Vector3f&, const Vector2f& rnd,
									  uint32& /*faceID*/, float& pdf) const override
	{
		PR_PROFILE_THIS;
		pdf = 1 / localSurfaceArea(0);
		return Vector3f(rnd(0), rnd(1), 0);
	}

	void provideGeometryPoint(const Vector3f& view, uint32 /*faceID*/, const Vector3f& parameter,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		float u = parameter[0];
		float v = parameter[1];

		float width = lerpWidth(u);
		Vector3f ny = mCurve.evalDerivative(u).cross(invTransform().linear() * view).normalized();
		pt.P		= mCurve.eval(u) + ny * (0.5f - v) * width;

		// Global
		pt.P = transform() * pt.P;
		pt.N = -view;
		Tangent::frame(pt.N, pt.Nx, pt.Ny);

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.UVW = Vector3f(u, v, 0.0f);

		pt.EntityID	   = id();
		pt.PrimitiveID = query.PrimitiveID;
		pt.MaterialID  = mMaterialID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = 0;
	}

	void beforeSceneBuild() override
	{
		PR_PROFILE_THIS;

		IEntity::beforeSceneBuild();

		mApproxSurfaceArea = 0.0f;
		constexpr size_t N = 50;
		for (size_t i = 0; i < N; ++i) {
			float a = i / (float)N;
			float b = (i + 1) / (float)N;

			Vector3f pA = mCurve.eval(a);
			Vector3f pB = mCurve.eval(b);

			float wA = lerpWidth(a);
			float wB = lerpWidth(b);

			float dist	= (pB - pA).norm();
			float qArea = std::min(wA, wB) * dist;
			float tArea = 0.5f * std::abs(wA - wB) * dist;
			mApproxSurfaceArea += qArea + tArea;
		}
	}

private:
	inline float lerpWidth(float t) const
	{
		return mWidth(0) * (1 - t) + mWidth(1) * t;
	}

	bool recursiveCheck(const Ray& in, HitPoint& out,
						const T& curve,
						float uMin, float uMax, size_t depth) const
	{
		PR_PROFILE_THIS;

		// Culling
		BoundingBox box;
		for (size_t i = 0; i < curve.degree() + 1; ++i)
			box.combine(curve.point(i));

		float width = std::max(lerpWidth(uMin), lerpWidth(uMax));
		box.expand(width / 2);

		box.intersects(Ray(Vector3f(0, 0, -1), Vector3f(0, 0, 1)), out);
		if (out.HitDistance == PR_INF)
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
			float denom	 = dir.block<2, 1>(0, 0).squaredNorm();
			if (std::abs(denom) < PR_EPSILON)
				return false;

			float w = -dir.block<2, 1>(0, 0).dot(s0.block<2, 1>(0, 0)) / denom;

			// U coordinate
			float u		   = std::max(uMin, std::min(uMax, uMin * (1 - w) + uMax * w));
			float hitwidth = lerpWidth(u);

			// Intersection point
			Vector3f p	= curve.eval(std::min(1.0f, std::max(0.0f, w)));
			float dist2 = p.block<2, 1>(0, 0).squaredNorm();
			if (dist2 > hitwidth * hitwidth * 0.25f)
				return false;

			if (p(2) < 0)
				return false;

			// V coordinate
			Vector3f dp = curve.evalDerivative(std::min(1.0f, std::max(0.0f, w)));
			float dist	= std::sqrt(dist2);
			float edge	= p(0) * dp(1) - dp(0) * p(1);
			float v		= (edge > 0)
						  ? 0.5f + dist / hitwidth
						  : 0.5f - dist / hitwidth;

			out.Parameter[0] = u;
			out.Parameter[1] = v;
			out.HitDistance	 = p(2);

			return true;
		}
	}

	uint32 mMaterialID;
	T mCurve;
	Vector2f mWidth;

	float mApproxSurfaceArea;
};

class CurveEntityPlugin : public IEntityPlugin {
public:
	std::shared_ptr<IEntity> create(uint32 id, const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();
		std::string name			 = params.getString("name", "__unnamed__");
		uint32 degree				 = (uint32)std::max(1, (int)params.getInt("degree", 3));

		std::vector<float> points = params.getNumberArray("points");
		if (points.size() != (degree + 1) * 3) {
			PR_LOG(L_ERROR) << "Curve " << name << " is invalid. Expected " << (degree + 1) * 3 << " entries in points" << std::endl;
			return nullptr;
		}

		Vector2f width = params.getVector2f("width", Vector2f(1, 1));
		
		const uint32 matID = ctx.lookupMaterialID(params.getParameter("material"));
		const uint32 emsID = ctx.lookupEmissionID(params.getParameter("emission"));

#define _CURVE(D)                                                                                                        \
	case D: {                                                                                                            \
		FixedCurve3<D + 1> curve;                                                                                        \
		for (size_t i = 0; i <= curve.degree(); ++i)                                                                     \
			curve.points()[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);                           \
		return std::make_shared<CurveEntity<FixedCurve3<D + 1>>>(id, name, ctx.transform(), curve, width, matID, emsID); \
	}

		switch (degree) {
			_CURVE(2)
			_CURVE(3)
			_CURVE(4)
			_CURVE(5)
			_CURVE(6)
			_CURVE(7)
			_CURVE(8)
			_CURVE(9)
		default: {
			Curve3::PointList list(degree + 1);
			for (size_t i = 0; i < list.size(); ++i)
				list[i] = Vector3f(points[3 * i], points[3 * i + 1], points[3 * i + 2]);

			return std::make_shared<CurveEntity<Curve3>>(id, name, ctx.transform(),
														 Curve3(list), width,
														 matID, emsID);
		}
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "curve", "bezier" });
		return names;
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::CurveEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)
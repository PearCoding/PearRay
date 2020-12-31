#pragma once

#include "Enum.h"
#include "ITransformable.h"
#include "geometry/BoundingBox.h"

namespace PR {
enum class EntityVisibility : uint8 {
	Camera = (uint8)RayFlag::Camera,
	Light  = (uint8)RayFlag::Light,
	Bounce = (uint8)RayFlag::Bounce,
	Shadow = (uint8)RayFlag::Shadow,
	All	   = Camera | Light | Bounce | Shadow
};
PR_MAKE_FLAGS(EntityVisibility, EntityVisibilityFlags)

struct GeometryPoint;
struct GeometryRepr;
struct GeometryDev;

struct PR_LIB_CORE EntityGeometryQueryPoint {
	Vector3f View;
	Vector3f Position;
	Vector2f UV;
	uint32 PrimitiveID;
	uint32 _padding;
};

struct PR_LIB_CORE EntitySamplePoint {
	Vector3f Position;
	Vector2f UV;
	uint32 PrimitiveID;
	float PDF_A;

	inline EntitySamplePoint(const Vector3f& p, const Vector2f& uv, uint32 primid, float pdf_a)
		: Position(p)
		, UV(uv)
		, PrimitiveID(primid)
		, PDF_A(pdf_a)
	{
	}

	inline EntityGeometryQueryPoint toQueryPoint(const Vector3f& view) const { return EntityGeometryQueryPoint{ view, Position, UV, PrimitiveID, 0 }; }
};

// Extra information useful for sampling
struct PR_LIB_CORE EntitySamplingInfo {
	Vector3f Origin; // Of surface point looking at entity
	Vector3f Normal; // Of surface point looking at entity
};

class PR_LIB_CORE IEntity : public ITransformable {
public:
	IEntity(uint32 emission_id, const std::string& name, const Transformf& transform);
	virtual ~IEntity();

	bool isRenderable() const override;

	inline EntityVisibilityFlags visibilityFlags() const;
	inline void setVisibilityFlags(EntityVisibilityFlags flags);

	// Optional Interface
	virtual bool isCollidable() const;
	virtual float collisionCost() const;

	virtual std::string dumpInformation() const override;

	// Mandatory Interface
	virtual float localSurfaceArea(uint32 materialID = PR_INVALID_ID) const = 0;
	virtual float worldSurfaceArea(uint32 materialID = PR_INVALID_ID) const { return volumeScalefactor() * this->localSurfaceArea(materialID); }

	virtual BoundingBox localBoundingBox() const = 0;
	inline BoundingBox worldBoundingBox() const;

	virtual GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const = 0;

	/// Sampling a point for NEE or similar where another surface is used as an observable point
	/// The default implementation just ignores the extra information
	virtual EntitySamplePoint sampleParameterPoint(const EntitySamplingInfo& info, const Vector2f& rnd) const
	{
		PR_UNUSED(info);
		return sampleParameterPoint(rnd);
	}
	virtual float sampleParameterPointPDF(const Vector3f& p, const EntitySamplingInfo& info) const
	{
		PR_UNUSED(p);
		PR_UNUSED(info);
		return sampleParameterPointPDF();
	}

	// Sampling a point on the entity without any additional information
	virtual EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const = 0;
	virtual float sampleParameterPointPDF() const
	{
		return 1.0f / localSurfaceArea();
	}

	virtual void provideGeometryPoint(const EntityGeometryQueryPoint& query, GeometryPoint& pt) const = 0;

	// Light
	inline bool hasEmission() const { return mEmissionID != PR_INVALID_ID; }
	inline uint32 emissionID() const { return mEmissionID; }

private:
	EntityVisibilityFlags mVisibilityFlags;
	const uint32 mEmissionID;
};
} // namespace PR

#include "IEntity.inl"

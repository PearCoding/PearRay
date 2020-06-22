#pragma once

#include "ITransformable.h"
#include "geometry/BoundingBox.h"

namespace PR {
enum EntityVisibilityFlag : uint8 {
	EVF_Camera = 0x1,
	EVF_Light  = 0x2,
	EVF_Bounce = 0x4,
	EVF_Shadow = 0x8,
	EVF_All	   = EVF_Camera | EVF_Light | EVF_Bounce | EVF_Shadow
};

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

struct PR_LIB_CORE EntityRandomPoint {
	Vector3f Position;
	Vector2f UV;
	uint32 PrimitiveID;
	float PDF_A; // Area

	inline EntityRandomPoint(const Vector3f& p, const Vector2f& uv, uint32 primid, float pdf)
		: Position(p)
		, UV(uv)
		, PrimitiveID(primid)
		, PDF_A(pdf)
	{
	}
};

class PR_LIB_CORE IEntity : public ITransformable {
public:
	IEntity(uint32 id, const std::string& name);
	virtual ~IEntity();

	bool isRenderable() const override;

	inline uint8 visibilityFlags() const;
	inline void setVisibilityFlags(uint8 flags);

	inline BoundingBox worldBoundingBox() const;

	// Optional Interface
	virtual bool isCollidable() const;
	virtual float collisionCost() const;

	virtual std::string dumpInformation() const override;

	// Mandatory Interface
	virtual bool isLight() const						   = 0;
	virtual float surfaceArea(uint32 materialID = 0) const = 0;

	virtual BoundingBox localBoundingBox() const = 0;

	virtual GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const = 0;

	virtual EntityRandomPoint pickRandomParameterPoint(const Vector3f& view, const Vector2f& rnd) const = 0;
	virtual void provideGeometryPoint(const EntityGeometryQueryPoint& query, GeometryPoint& pt) const	= 0;

	// IObject
	virtual void beforeSceneBuild() override;

private:
	uint8 mVisibilityFlags;
	BoundingBox calcWorldBoundingBox() const;
	BoundingBox mWorldBoundingBox_Cache;
};
} // namespace PR

#include "IEntity.inl"

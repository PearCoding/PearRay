#pragma once

#include "IFreezable.h"
#include "math/SIMD.h"
#include "shader/ShadingGroup.h"
#include "shader/ShadingPoint.h"
#include <memory>

namespace PR {
class RenderTileSession;

/* A material having a diffuse path should never have a specular path and vice versa! */
enum MaterialScatteringType : uint32 {
	MST_DiffuseReflection = 0,
	MST_SpecularReflection,
	MST_DiffuseTransmission,
	MST_SpecularTransmission
};

// Evaluation
struct PR_LIB_INLINE MaterialEvalInput {
	ShadingPoint Point;
	Vector3f Outgoing;
};

struct PR_LIB_INLINE MaterialEvalOutput {
	float Weight;
	float PDF_S_Forward;
	float PDF_S_Backward;
};

// Sampling
struct PR_LIB_INLINE MaterialSampleInput {
	ShadingPoint Point;
	Vector2f RND;
};

struct PR_LIB_INLINE MaterialSampleOutput {
	Vector3f Outgoing;
	float Weight;
	float PDF_S_Forward;
	float PDF_S_Backward;
	MaterialScatteringType Type;
};

class PR_LIB IMaterial : public IFreezable {
public:
	IMaterial(uint32 id);
	virtual ~IMaterial() {}

	inline uint32 id() const;

	virtual void startGroup(size_t size, const RenderTileSession& session) = 0;
	virtual void endGroup()												   = 0;

	/*
		Evaluate the BxDF based on incident and outgoing direction and point information.
	*/
	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderTileSession& session) const = 0;

	/*
		Sample a direction and evaluate.
	*/
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderTileSession& session) const = 0;

	inline bool canBeShaded() const;
	inline void enableShading(bool b);

	inline void enableShadow(bool b);
	inline bool allowsShadow() const;

	inline void enableSelfShadow(bool b);
	inline bool allowsSelfShadow() const;

	inline void enableCameraVisibility(bool b);
	inline bool isCameraVisible() const;

	virtual std::string dumpInformation() const;

private:
	bool mCanBeShaded;
	bool mShadow;
	bool mSelfShadow;
	bool mCameraVisible;
	uint32 mID;
};
} // namespace PR

#include "IMaterial.inl"

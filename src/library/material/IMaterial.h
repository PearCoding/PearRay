#pragma once

#include "IFreezable.h"
#include "shader/ShadingGroup.h"
#include "math/SIMD.h"
#include <memory>

namespace PR {
class RenderSession;

/* A material having a diffuse path should never have a specular path and vice versa! */
enum MaterialScatteringType : uint32 {
	MST_DiffuseReflection = 0,
	MST_SpecularReflection,
	MST_DiffuseTransmission,
	MST_SpecularTransmission
};

// Evaluation
struct PR_LIB_INLINE MaterialEvalInput {
	uint32 EntityID;// Always just one entity

	vfloat Incident[3];
	vfloat Outgoing[3];
	vfloat UV[2];
	vuint32 WavelengthIndex;
	vuint32 PrimitiveID;
};

struct PR_LIB_INLINE MaterialEvalOutput {
	vfloat Weight;
	vfloat PDF_S_Forward;
	vfloat PDF_S_Backward;
};

// Sampling
struct PR_LIB_INLINE MaterialSampleInput {
	uint32 EntityID;// Always just one entity

	vfloat RND[2];
	vfloat Incident[3];
	vfloat UV[2];
	vuint32 WavelengthIndex;
	vuint32 PrimitiveID;
};

struct PR_LIB_INLINE MaterialSampleOutput {
	vfloat Outgoing[3];
	vfloat Weight;
	vfloat PDF_S_Forward;
	vfloat PDF_S_Backward;
	vuint32 Type;
};

class PR_LIB IMaterial : public IFreezable {
public:
	IMaterial(uint32 id);
	virtual ~IMaterial() {}

	inline uint32 id() const;

	virtual void startGroup(size_t size, const RenderSession& session) = 0;
	virtual void endGroup()											   = 0;

	/*
		Evaluate the BxDF based on incident and outgoing direction and point information.
	*/
	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderSession& session) const = 0;

	/*
		Sample a direction and evaluate.
	*/
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderSession& session) const = 0;

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

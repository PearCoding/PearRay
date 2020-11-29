#pragma once

#include "entity/ITransformable.h"
#include "trace/IntersectionPoint.h"

namespace PR {
struct PR_LIB_CORE InfiniteLightEvalInput {
	SpectralBlob WavelengthNM;
	Vector3f Direction;
	uint32 IterationDepth;
};

struct PR_LIB_CORE InfiniteLightEvalOutput {
	SpectralBlob Radiance;
	float Direction_PDF_S;
};

struct PR_LIB_CORE InfiniteLightSampleDirInput {
	Vector2f DirectionRND;
	SpectralBlob WavelengthNM;
};

struct PR_LIB_CORE InfiniteLightSampleDirOutput {
	SpectralBlob Radiance;
	float Direction_PDF_S;
	Vector3f Outgoing;
};

struct PR_LIB_CORE InfiniteLightSamplePosDirInput : public InfiniteLightSampleDirInput {
	Vector2f PositionRND;
	const IntersectionPoint* Point = nullptr;
};

struct PR_LIB_CORE InfiniteLightSamplePosDirOutput : public InfiniteLightSampleDirOutput {
	Vector3f LightPosition;
	float Position_PDF_A;
};

class RenderTileSession;
class PR_LIB_CORE IInfiniteLight : public ITransformable {
public:
	IInfiniteLight(const std::string& name, const Transformf& transform);
	virtual ~IInfiniteLight() {}

	virtual bool hasDeltaDistribution() const { return false; }

	virtual void eval(const InfiniteLightEvalInput& in, InfiniteLightEvalOutput& out, const RenderTileSession& session) const = 0;
	/// Sample the light and generating a direction towards the infinite light source.
	virtual void sampleDir(const InfiniteLightSampleDirInput& in, InfiniteLightSampleDirOutput& out, const RenderTileSession& session) const = 0;
	/// Sample the light with light position and direction towards the newly sampled light position of the infinite light source.
	virtual void samplePosDir(const InfiniteLightSamplePosDirInput& in, InfiniteLightSamplePosDirOutput& out, const RenderTileSession& session) const = 0;

	/// Return average power (W/m^2) for given wavelengths
	virtual SpectralBlob power(const SpectralBlob& wvl) const = 0; // Average (W/m^2)

	/// Dump information
	virtual std::string dumpInformation() const;
};
} // namespace PR

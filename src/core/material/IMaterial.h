#pragma once

#include "IObject.h"
#include "MaterialData.h"

namespace PR {
class RenderTileSession;

enum MaterialFlags {
	MF_DeltaDistribution = 0x1,
	MF_SpectralVarying	 = 0x2 // The sampled direction (not the weight) is different for each wavelength
};

class PR_LIB_CORE IMaterial : public IObject {
public:
	IMaterial(uint32 id);
	virtual ~IMaterial() {}

	inline uint32 id() const;

	virtual void startGroup(size_t size, const RenderTileSession& session)
	{
		PR_UNUSED(size);
		PR_UNUSED(session);
	}
	virtual void endGroup() {}

	/*
		Evaluate the BxDF based on incident and outgoing direction and point information.
		The calculation is in shading space.
	*/
	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderTileSession& session) const = 0;

	/*
		Sample a direction and evaluate.
		The calculation and output is in shading space.
	*/
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderTileSession& session) const = 0;

	virtual int flags() const { return 0; }

	inline bool hasDeltaDistribution() const { return this->flags() & MF_DeltaDistribution; }
	inline bool isSpectralVarying() const { return this->flags() & MF_SpectralVarying; }

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

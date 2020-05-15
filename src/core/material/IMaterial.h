#pragma once

#include "IObject.h"
#include "MaterialData.h"

namespace PR {
class RenderTileSession;
class PR_LIB_CORE IMaterial : public IObject {
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

	virtual bool hasDeltaDistribution() const { return false; }

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
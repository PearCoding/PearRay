#pragma once

#include "Enum.h"
#include "MaterialData.h"

namespace PR {
class RenderTileSession;

enum class MaterialFlag {
	OnlyDeltaDistribution = 0x1,
	HasFlourescence		  = 0x2
};
PR_MAKE_FLAGS(MaterialFlag, MaterialFlags)

class PR_LIB_CORE IMaterial {
public:
	IMaterial();
	virtual ~IMaterial() {}

	/// Evaluate the BxDF based on incident and outgoing direction and point information.
	/// The calculation is in shading space.
	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderTileSession& session) const = 0;

	/// Compute the pdf of the BxDF based on incident and outgoing direction and point information.
	/// The calculation is in shading space.
	virtual void pdf(const MaterialEvalInput& in, MaterialPDFOutput& out, const RenderTileSession& session) const = 0;

	/// Sample a direction and evaluate.
	/// The calculation and output is in shading space.
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderTileSession& session) const = 0;

	virtual MaterialFlags flags() const { return 0; }
	inline bool hasOnlyDeltaDistribution() const { return flags() & MaterialFlag::OnlyDeltaDistribution; }
	inline bool hasFlourescence() const { return flags() & MaterialFlag::HasFlourescence; }

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
};
} // namespace PR

#include "IMaterial.inl"

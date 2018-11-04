#pragma once

#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "shader/ShadingSocket.h"

namespace PR {
class LambertMaterial : public IMaterial {
public:
	LambertMaterial(uint32 id);
	virtual ~LambertMaterial() = default;

	std::shared_ptr<FloatSpectralShadingSocket> albedo() const;
	void setAlbedo(const std::shared_ptr<FloatSpectralShadingSocket>& diffSpec);

	virtual void startGroup(size_t size, const RenderTileSession& session);
	virtual void endGroup();

	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderTileSession& session) const;
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderTileSession& session) const;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	std::shared_ptr<FloatSpectralShadingSocket> mAlbedo;
};
}

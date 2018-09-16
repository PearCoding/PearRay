#pragma once

#include "material/IMaterial.h"
#include "material/IMaterialFactory.h"
#include "shader/ShaderOutput.h"

namespace PR {
class LambertMaterial : public IMaterial {
public:
	LambertMaterial(uint32 id);
	virtual ~LambertMaterial() = default;

	std::shared_ptr<SpectrumShaderOutput> albedo() const;
	void setAlbedo(const std::shared_ptr<SpectrumShaderOutput>& diffSpec);

	virtual void startGroup(size_t size, const RenderSession& session);
	virtual void endGroup();

	virtual void eval(const MaterialEvalInput& in, MaterialEvalOutput& out, const RenderSession& session) const;
	virtual void sample(const MaterialSampleInput& in, MaterialSampleOutput& out, const RenderSession& session) const;

	std::string dumpInformation() const override;

protected:
	void onFreeze(RenderContext* context) override;

private:
	std::shared_ptr<SpectrumShaderOutput> mAlbedo;
};
}

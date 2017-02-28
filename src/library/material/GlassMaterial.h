#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial(uint32 id);

		bool isThin() const;
		void setThin(bool b);

		const std::shared_ptr<SpectralShaderOutput>& specularity() const;
		void setSpecularity(const std::shared_ptr<SpectralShaderOutput>& spec);

		const std::shared_ptr<SpectralShaderOutput>& indexData() const;
		void setIndexData(const std::shared_ptr<SpectralShaderOutput>& data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& path_weight, uint32 path) override;
		uint32 samplePathCount() const override;

		virtual std::string dumpInformation() const override;
	private:
		std::shared_ptr<SpectralShaderOutput> mSpecularity;
		std::shared_ptr<SpectralShaderOutput> mIndex;
		bool mThin;
	};
}

#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GlassMaterial : public Material
	{
	public:
		GlassMaterial(uint32 id);

		bool sampleIOR() const;
		void setSampleIOR(bool b);

		bool isThin() const;
		void setThin(bool b);
		
		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		SpectralShaderOutput* indexData() const;
		void setIndexData(SpectralShaderOutput* data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;
	
		PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, Spectrum& path_weight, uint32 path) override;
		uint32 samplePathCount() const override;

		void setup(RenderContext* context) override;
		virtual std::string dumpInformation() const override;
	private:
		SpectralShaderOutput* mSpecularity;
		SpectralShaderOutput* mIndex;
		bool mSampleIOR;
		bool mThin;

		// Cache:
		uint32 mIntervalCount_Cache;
		uint32 mIntervalLength_Cache;
		uint32 mPathCount_Cache;
	};
}
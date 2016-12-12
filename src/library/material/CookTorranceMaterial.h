#pragma once

#include "Material.h"

namespace PR
{	
	class PR_LIB CookTorranceMaterial : public Material
	{
	public:
		enum FresnelMode
		{
			FM_Dielectric,
			FM_Conductor
		};

		enum DistributionMode
		{
			DM_Blinn,
			DM_Beckmann,
			DM_GGX
		};

		enum GeometryMode
		{
			GM_Implicit,
			GM_Neumann,
			GM_CookTorrance,
			GM_Kelemen
		};

		CookTorranceMaterial(uint32 id);

		// Modes
		FresnelMode fresnelMode() const;
		void setFresnelMode(FresnelMode mode);

		DistributionMode distributionMode() const;
		void setDistributionMode(DistributionMode mode);

		GeometryMode geometryMode() const;
		void setGeometryMode(GeometryMode mode);

		// Diffuse
		SpectralShaderOutput* albedo() const;
		void setAlbedo(SpectralShaderOutput* diffSpec);

		ScalarShaderOutput* diffuseRoughness() const;
		void setDiffuseRoughness(ScalarShaderOutput* data);

		// Specular
		SpectralShaderOutput* specularity() const;
		void setSpecularity(SpectralShaderOutput* spec);

		ScalarShaderOutput* specularRoughnessX() const;
		void setSpecularRoughnessX(ScalarShaderOutput* data);

		ScalarShaderOutput* specularRoughnessY() const;
		void setSpecularRoughnessY(ScalarShaderOutput* data);

		SpectralShaderOutput* ior() const;
		void setIOR(SpectralShaderOutput* data);

		SpectralShaderOutput* conductorAbsorption() const;
		void setConductorAbsorption(SpectralShaderOutput* data);

		ScalarShaderOutput* reflectivity() const;
		void setReflectivity(ScalarShaderOutput* data);

		Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override;
		PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override;

		PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, Spectrum& path_weight, uint32 path) override;
		uint32 samplePathCount() const override;

		virtual std::string dumpInformation() const override;
	private:
		PM::vec3 diffuse_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);
		PM::vec3 specular_path(const ShaderClosure& point, const PM::vec3& rnd, float& pdf);
		
		FresnelMode mFresnelMode;
		DistributionMode mDistributionMode;
		GeometryMode mGeometryMode;

		SpectralShaderOutput* mAlbedo;
		ScalarShaderOutput* mDiffuseRoughness;

		SpectralShaderOutput* mSpecularity;
		ScalarShaderOutput* mSpecRoughnessX;
		ScalarShaderOutput* mSpecRoughnessY;
		SpectralShaderOutput* mIOR;
		SpectralShaderOutput* mConductorAbsorption;

		ScalarShaderOutput* mReflectivity;
	};
}
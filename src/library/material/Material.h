#pragma once

#include "shader/ShaderOutput.h"
#include <memory>

namespace PR {
class RenderEntity;
struct ShaderClosure;
class Ray;
class RenderContext;

/* A material having a diffuse path should never have a specular path and vice versa! */
enum MaterialScatteringType {
	MST_DiffuseReflection = 0,
	MST_SpecularReflection,
	MST_DiffuseTransmission,
	MST_SpecularTransmission
};

struct MaterialSample {
	float PDF_S{ 0 }; // Respect to Solid Angle
	Eigen::Vector3f L;
	MaterialScatteringType ScatteringType{ MST_DiffuseReflection };

	inline bool isSpecular() const
	{
		return ScatteringType == MST_SpecularReflection || ScatteringType == MST_SpecularTransmission;
	}

	inline bool isReflection() const
	{
		return ScatteringType == MST_SpecularReflection || ScatteringType == MST_DiffuseReflection;
	}
};

class PR_LIB Material {
public:
	explicit Material(uint32 id);
	virtual ~Material() {}

	/*
		 Evaluate the BxDF based on L and point information.
		*/
	virtual Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) = 0;

	/*
		 Calculate the PDF based on L. Can be infinitive to force predestined directions (e.g. glass)
		*/
	virtual float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) = 0;

	/*
		 Sample a direction based on the uniform rnd value. (Russian Roulette)
		*/
	virtual MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) = 0;

	/*
		Sample a direction based on the uniform rnd value. (Non russian roulette)
	*/
	virtual MaterialSample samplePath(const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path)
	{
		return sample(point, rnd);
	}

	/*
		Returns amount of special paths in the material. (like reflection/refraction)
	*/
	virtual uint32 samplePathCount() const
	{
		return 1;
	}

	inline uint32 id() const;

	inline bool isLight() const;

	inline const std::shared_ptr<SpectralShaderOutput>& emission() const;
	inline void setEmission(const std::shared_ptr<SpectralShaderOutput>& spec);

	inline bool canBeShaded() const;
	inline void enableShading(bool b);

	inline void enableShadow(bool b);
	inline bool allowsShadow() const;

	inline void enableSelfShadow(bool b);
	inline bool allowsSelfShadow() const;

	inline void enableCameraVisibility(bool b);
	inline bool isCameraVisible() const;

	virtual void setup(RenderContext* context) {}

	virtual std::string dumpInformation() const;

private:
	std::shared_ptr<SpectralShaderOutput> mEmission;

	const uint32 mID;
	bool mCanBeShaded;
	bool mShadow;
	bool mSelfShadow;
	bool mCameraVisible;
};
}

#include "Material.inl"

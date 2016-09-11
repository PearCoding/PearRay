#include "DebugBoundingBoxMaterial.h"
#include "ray/Ray.h"
#include "shader/ShaderClosure.h"

namespace PR
{
	DebugBoundingBoxMaterial::DebugBoundingBoxMaterial() :
		Material(), mDensity(0.5f)
	{
	}

	Spectrum DebugBoundingBoxMaterial::apply(const ShaderClosure& point, const PM::vec3& L)
	{
		return mColor*mDensity;
	}

	void DebugBoundingBoxMaterial::setColor(const Spectrum& spec)
	{
		mColor = spec;
	}

	Spectrum DebugBoundingBoxMaterial::color() const
	{
		return mColor;
	}

	void DebugBoundingBoxMaterial::setDensity(float f)
	{
		mDensity = f;
	}

	float DebugBoundingBoxMaterial::density() const
	{
		return mDensity;
	}

	float DebugBoundingBoxMaterial::pdf(const ShaderClosure& point, const PM::vec3& L)
	{
		return std::numeric_limits<float>::infinity();
	}

	PM::vec3 DebugBoundingBoxMaterial::sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf)
	{
		pdf = DebugBoundingBoxMaterial::pdf(point, point.V);
		return point.V;
	}
}
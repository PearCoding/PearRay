#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB NormalDebugMaterial : public Material
	{
	public:
		NormalDebugMaterial();

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness(const FacePoint& point) const override;
	};
}
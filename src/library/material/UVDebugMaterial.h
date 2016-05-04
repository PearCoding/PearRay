#pragma once

#include "Material.h"

namespace PR
{
	class PR_LIB UVDebugMaterial : public Material
	{
	public:
		UVDebugMaterial();

		void apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
			Spectrum& diff, Spectrum& spec) override;

		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness() const override;
	};
}
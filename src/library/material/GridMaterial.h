#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GridMaterial : public Material
	{
	public:
		GridMaterial();

		void setFirstMaterial(Material* mat);
		Material* firstMaterial() const;

		void setSecondMaterial(Material* mat);
		Material* secondMaterial() const;

		void setGridCount(int i);
		int gridCount() const;

		void apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li,
			Spectrum& diff, Spectrum& spec) override;
		
		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness() const override;
	private:
		Material* mFirst;
		Material* mSecond;

		int mGridCount;
	};
}
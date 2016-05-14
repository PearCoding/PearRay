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

		void setTileUV(bool b);
		bool tileUV() const;

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		
		float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;
		float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) override;

		float roughness(const FacePoint& point) const override;
	private:
		FacePoint applyGrid(const FacePoint& point, int& u, int& v) const;

		Material* mFirst;
		Material* mSecond;

		int mGridCount;

		bool mTiledUV;
	};
}
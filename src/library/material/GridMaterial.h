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

		Spectrum apply(const SamplePoint& point, const PM::vec3& L) override;
		float pdf(const SamplePoint& point, const PM::vec3& L) override;
		PM::vec3 sample(const SamplePoint& point, const PM::vec3& rnd, float& pdf) override;

	private:
		SamplePoint applyGrid(const SamplePoint& point, int& u, int& v) const;

		Material* mFirst;
		Material* mSecond;

		int mGridCount;

		bool mTiledUV;
	};
}
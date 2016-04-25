#pragma once

#include "Material.h"
#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB GridMaterial : public Material
	{
	public:
		GridMaterial();

		bool isLight() const override;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void setFirstMaterial(Material* mat);
		Material* firstMaterial() const;

		void setSecondMaterial(Material* mat);
		Material* secondMaterial() const;

		void setGridCount(int i);
		int gridCount() const;

		void apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer) override;
	private:
		Material* mFirst;
		Material* mSecond;

		int mGridCount;
		bool mCameraVisible;
	};
}
#pragma once

#include "material/Material.h"
#include "spectral/Spectrum.h"

namespace PRU
{
	class PR_LIB_UTILS GSLMaterial : public PR::Material
	{
	public:
		GSLMaterial();

		void enableLight(bool b);
		bool isLight() const;

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableSelfShadow(bool b);
		bool canBeSelfShadowed() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

		void apply(PR::Ray& in, PR::RenderEntity* entity, const PR::FacePoint& point, PR::Renderer* renderer);
	private:
		bool mCanBeShaded;
		bool mLight;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}
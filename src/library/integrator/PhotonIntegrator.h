#pragma once

#include "Integrator.h"

namespace PR
{
	namespace Photon
	{
		class PhotonMap;
		struct PhotonSphere;
	}

	class PR_LIB PhotonIntegrator : public Integrator
	{
	public:
		PhotonIntegrator();
		~PhotonIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) override;

	private:
		Photon::PhotonMap* mMap;
		Photon::PhotonSphere* mPhotonSpheres;
		uint32 mSphereCount;
	};
}
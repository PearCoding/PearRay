#pragma once

#include "Affector.h"

namespace PR
{
	namespace Photon
	{
		class PhotonMap;
		struct PhotonSphere;
	}

	class PR_LIB PhotonAffector : public Affector
	{
	public:
		PhotonAffector();
		~PhotonAffector();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) override;

	private:
		Photon::PhotonMap* mMap;
		Photon::PhotonSphere* mPhotonSpheres;
		uint32 mSphereCount;
	};
}
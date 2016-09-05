#pragma once

#include "Photon.h"

#include "PearMath.h"
#include "geometry/BoundingBox.h"

#include <functional>

namespace PR
{
	namespace Photon
	{
		// Based on the implementation in the book:
		// Realistic Image Synthesis Using Photon Mapping (2nd Edition: 2001)
		// from Henrik Wann Jensen
		class PhotonMap
		{
			PR_CLASS_NON_COPYABLE(PhotonMap);
		public:
			PhotonMap(uint64 max_photons);
			~PhotonMap();

			void reset();

			inline PM::vec3 photonDirection(const Photon* p)// Standard spherical coordinates
			{
				return PM::pm_Set(mSinTheta[p->Theta] * mCosPhi[p->Phi],
					mSinTheta[p->Theta] * mSinPhi[p->Phi],
					mCosPhi[p->Phi]);
			}

			inline bool isFull() const { return mStoredPhotons >= mMaxPhotons; }
			inline bool isEmpty() const { return mStoredPhotons == 0; }
			inline uint64 storedPhotons() const { return mStoredPhotons; }

			void locateSphere(PhotonSphere& sphere, uint64 index);
			void locateDome(PhotonSphere& sphere, uint64 index);

			typedef std::function<bool(const Photon*, const PhotonSphere&, float&)> CheckFunction;
			void locate(PhotonSphere& sphere, uint64 index, CheckFunction checkFunc);

			void store(const Spectrum& spec, const PM::vec3& pos, const PM::vec3& dir, float pdf);
			void scalePhotonPower(float scale);

			void balanceTree();// Balance the KD-tree before using

		private:
			// KD-tree utils
			void balanceSegment(Photon** balance, Photon** original, uint64 index, uint64 start, uint64 end);
			static void medianSplit(Photon** photon, uint64 start, uint64 end, uint64 median, int axis);

			Photon* mPhotons;
			uint64 mStoredPhotons;
			uint64 mHalfStoredPhotons;
			const uint64 mMaxPhotons;
			uint64 mPreviousScaleIndex;// Contains the end index of the last scale

			// Cache:
			float mCosTheta[256];
			float mSinTheta[256];
			float mCosPhi[256];
			float mSinPhi[256];

			BoundingBox mBox;
		};
	}
}
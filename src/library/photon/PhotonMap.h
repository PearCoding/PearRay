#pragma once

#include "Photon.h"

#include "PearMath.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	namespace Photon
	{
		// Based on the implementation in the book:
		// Realistic Image Synthesis Using Photon Mapping (2nd Edition: 2001)
		// from Henrik Wann Jensen
		// TODO: This is just the starting point to implement progressive photon mapping later
		class PhotonMap
		{
		public:
			PhotonMap(uint64 max_photons);
			~PhotonMap();

			PM::vec3 photonDirection(const Photon* p);

			bool isFull() const;
			void store(const Spectrum& spec, const PM::vec3& pos, const PM::vec3& dir);
			void scalePhotonPower(float scale);

			void balanceTree();// Balance the KD-tree before using

			//TODO: Add estimation

		private:
			// KD-tree utils
			void balanceSegment(Photon** balance, Photon** original, uint64 index, uint64 start, uint64 end);
			void medianSplit(Photon** photon, uint64 start, uint64 end, uint64 median, int axis);

			Photon* mPhotons;
			uint64 mStoredPhotons;
			uint64 mHalfStoredPhotons;
			uint64 mMaxPhotons;
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
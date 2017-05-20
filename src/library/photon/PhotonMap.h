#pragma once

#include "geometry/BoundingBox.h"
#include "spectral/Spectrum.h"
#include "math/Generator.h"

#include "photon/Photon.h"

#include <atomic>
#include <tbb/concurrent_hash_map.h>
#include <vector>

//#define PR_USE_APPROX_PHOTON_MAP

namespace PR
{
	namespace Photon
	{
		struct PhotonSphere // Setup for the estimation query
		{
			uint64 MaxPhotons;
			Eigen::Vector3f Normal;
			float SqueezeWeight;
			Eigen::Vector3f Center;
			float Distance2;
		};

		// Spatial Hashmap
		class PhotonMap
		{
			PR_CLASS_NON_COPYABLE(PhotonMap);
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			typedef bool (*CheckFunction)(const Photon&, const PhotonSphere&, float&);

			inline PhotonMap(float gridDelta);
			inline ~PhotonMap();

			inline void reset();

			inline bool isEmpty() const { return mStoredPhotons == 0; }
			inline uint64 storedPhotons() const { return mStoredPhotons; }

			template<typename AccumFunction>
			inline Spectrum estimateSphere(const PhotonSphere& sphere, AccumFunction accumFunc, size_t& found) const;

			template<typename AccumFunction>
			inline Spectrum estimateDome(const PhotonSphere& sphere, AccumFunction accumFunc, size_t& found) const;

			template<typename AccumFunction>
			inline Spectrum estimate(const PhotonSphere& sphere, CheckFunction checkFunc, AccumFunction accumFunc, size_t& found) const;

			inline void mapDirection(const Eigen::Vector3f& dir, uint8& theta, uint8& phi) const
			{
				int theta2 = (int)(std::cos(dir(2)) * 256 * PR_1_PI);
				if (theta2 > 255)
					theta = 255;
				else
					theta = (uint8)theta2;

				int phi2 = (int)(std::atan2(dir(1), dir(0)) * 256 * PR_1_PI * 0.5f);
				if (phi2 > 255)
					phi = 255;
				else
					phi = (uint8)phi2;
			}

			inline Eigen::Vector3f evalDirection(uint8 theta, uint8 phi) const
			{
				return Eigen::Vector3f(
					mSinTheta[theta] * mCosPhi[phi],
					mSinTheta[theta] * mSinPhi[phi],
					mCosPhi[phi]);
			}

			inline void store(const Eigen::Vector3f& pos, const Photon& point);

		private:
			struct KeyCoord
			{
				int32 X, Y, Z;

				inline bool operator ==(const KeyCoord& other) const;
			};

			inline KeyCoord toCoords(const Eigen::Vector3f& v) const;

			struct hash_compare
			{
				inline static size_t hash(const KeyCoord&);
				inline static bool equal(const KeyCoord& k1, const KeyCoord& k2);
			};

#ifdef PR_USE_APPROX_PHOTON_MAP
			struct ApproxPhoton
			{
				Photon Approximation;
				uint32 Count;
				inline ApproxPhoton() : Count(0) {}
			};
			typedef tbb::concurrent_hash_map<KeyCoord, ApproxPhoton, hash_compare> Map;
#else
			typedef tbb::concurrent_hash_map<KeyCoord, std::vector<Photon>, hash_compare> Map;
#endif
			Map mPhotons;

			std::atomic<uint64> mStoredPhotons;
			const float mGridDelta;

			// Cache:
			float mInvGridDelta;
			float mCosTheta[256];
			float mSinTheta[256];
			float mCosPhi[256];
			float mSinPhi[256];

			BoundingBox mBox;
		};
	}
}

#include "PhotonMap.inl"

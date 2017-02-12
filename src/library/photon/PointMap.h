#pragma once

#include "geometry/BoundingBox.h"
#include "spectral/Spectrum.h"
#include "math/Generator.h"

#include "photon/Photon.h"

#include <tbb/concurrent_hash_map.h>
#include <vector>

#define PR_USE_APPROX_PHOTON_MAP

namespace PR
{
	namespace Photon
	{
		struct PointSphere // Setup for the estimation query
		{
			uint64 MaxPhotons;
			PM::vec3 Normal;
			float SqueezeWeight;
			PM::vec3 Center;
			float Distance2;
		};

		// Spatial Hashmap
		class PointMap
		{
			PR_CLASS_NON_COPYABLE(PointMap);
		public:
			typedef bool (*CheckFunction)(const Photon&, const PointSphere&, float&);

			inline PointMap(float gridDelta);
			inline ~PointMap();

			inline void reset();

			inline bool isEmpty() const { return mStoredPhotons == 0; }
			inline uint64 storedPhotons() const { return mStoredPhotons; }

			template<typename AccumFunction>
			inline Spectrum estimateSphere(const PointSphere& sphere, AccumFunction accumFunc, size_t& found) const;

			template<typename AccumFunction>
			inline Spectrum estimateDome(const PointSphere& sphere, AccumFunction accumFunc, size_t& found) const;

			template<typename AccumFunction>
			inline Spectrum estimate(const PointSphere& sphere, CheckFunction checkFunc, AccumFunction accumFunc, size_t& found) const;

			inline void mapDirection(const PM::vec3& dir, uint8& theta, uint8& phi) const
			{
				int theta2 = (int)(PM::pm_SafeACos(PM::pm_GetZ(dir)) * 256 * PM_INV_PI_F);
				if (theta2 > 255)
					theta = 255;
				else
					theta = (uint8)theta2;

				int phi2 = (int)(std::atan2(PM::pm_GetY(dir), PM::pm_GetX(dir)) * 256 * PM_INV_PI_F * 0.5f);
				if (phi2 > 255)
					phi = 255;
				else
					phi = (uint8)phi2;
			}

			inline PM::vec3 evalDirection(uint8 theta, uint8 phi) const
			{
				return PM::pm_Set(
					mSinTheta[theta] * mCosPhi[phi],
					mSinTheta[theta] * mSinPhi[phi],
					mCosPhi[phi]);
			}

			inline void store(const PM::vec3& pos, const Photon& point);

		private:
			struct KeyCoord
			{
				int32 X, Y, Z;

				inline bool operator ==(const KeyCoord& other) const;
			};

			inline KeyCoord toCoords(const PM::vec3& v) const;

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

			uint64 mStoredPhotons;
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

#include "PointMap.inl"

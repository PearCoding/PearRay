#pragma once

#include "Config.h"
#include "PearMath.h"
#include "geometry/BoundingBox.h"

namespace PR
{
	namespace Photon
	{
		template<class T>
		struct PointSphere // Contains the photons in radius around the center
		{
			uint32 Max;
			uint32 Found;
			bool GotHeap;
			PM::vec3 Normal;
			float SqueezeWeight;
			PM::vec3 Center;
			float* Distances2;
			const T** Index;
		};

		// Based on the implementation in the book:
		// Realistic Image Synthesis Using Photon Mapping (2nd Edition: 2001)
		// from Henrik Wann Jensen

		/* T must have attributes:
		 *   float Position[3];
		 *   uint8 KDFlags;
		 */
		template<class T>
		class PointMap
		{
			PR_CLASS_NON_COPYABLE(PointMap);
		public:
			PointMap(uint64 max_points);
			~PointMap();

			inline void reset();

			inline bool isFull() const { return mStoredPhotons >= mMaxPhotons; }
			inline bool isEmpty() const { return mStoredPhotons == 0; }
			inline uint64 storedPhotons() const { return mStoredPhotons; }

			inline void locateSphere(PointSphere<T>& sphere, uint64 index);
			inline void locateDome(PointSphere<T>& sphere, uint64 index);

			typedef bool (*CheckFunction)(const T*, const PointSphere<T>&, float&);
			inline void locate(PointSphere<T>& sphere, uint64 index, CheckFunction checkFunc);

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
				return PM::pm_Set(mSinTheta[theta] * mCosPhi[phi],
					mSinTheta[theta] * mSinPhi[phi],
					mCosPhi[phi]);
			}

			inline void store(const PM::vec3& pos, const T& point);

			template<typename CallFunction>
			inline void callNewestPoints(CallFunction f);

			inline void balanceTree();// Balance the KD-tree before using

		private:
			// KD-tree utils
			void balanceSegment(T** balance, T** original, uint64 index, uint64 start, uint64 end);
			static void medianSplit(T** photon, uint64 start, uint64 end, uint64 median, int axis);

			T* mPhotons;
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

#include "PointMap.inl"
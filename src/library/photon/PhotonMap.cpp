#include "PhotonMap.h"


namespace PR
{
	namespace Photon
	{
		PhotonMap::PhotonMap(uint64 max_photons) :
			mPhotons(nullptr), mStoredPhotons(0), mHalfStoredPhotons(0), mPreviousScaleIndex(1), mMaxPhotons(max_photons)
		{
			mPhotons = new Photon[max_photons + 1];
			PR_ASSERT(mPhotons);

			// Caching
			for (int i = 0; i < 256; ++i)
			{
				float angle = i * (1.0f / 256)*PM_PI_F;

				PM::pm_SinCosT(angle, mSinTheta[i], mCosTheta[i]);
				PM::pm_SinCosT(2 * angle, mSinPhi[i], mCosPhi[i]);
			}
		}

		PhotonMap::~PhotonMap()
		{
			delete[] mPhotons;
		}

		PM::vec3 PhotonMap::photonDirection(const Photon* p)// Standard spherical coordinates
		{
			return PM::pm_Set(mSinTheta[p->Theta] * mCosPhi[p->Phi],
				mSinTheta[p->Theta] * mSinPhi[p->Phi],
				mCosPhi[p->Phi]);
		}

		bool PhotonMap::isFull() const
		{
			return mStoredPhotons >= mMaxPhotons;
		}

		void PhotonMap::store(const Spectrum& spec, const PM::vec3& pos, const PM::vec3& dir)
		{
			if (isFull())
				return;

			mStoredPhotons++;
			Photon* node = &mPhotons[mStoredPhotons];

			mBox.put(pos);
			node->Position[0] = PM::pm_GetX(pos);
			node->Position[1] = PM::pm_GetY(pos);
			node->Position[2] = PM::pm_GetZ(pos);

			int theta = (int)(std::acos(PM::pm_GetZ(dir)) * 256 * PM_INV_PI_F);
			if (theta > 255)
				node->Theta = 255;
			else
				node->Theta = (uint8)theta;

			int phi = (int)(std::atan2(PM::pm_GetY(dir), PM::pm_GetX(dir)) * 256 * PM_INV_PI_F * 0.5f);
			if (phi > 255)
				node->Phi = 255;
			else
				node->Phi = (uint8)phi;
		}

		void PhotonMap::scalePhotonPower(float scale)
		{
			for (uint64 i = mPreviousScaleIndex; i <= mStoredPhotons; ++i)
			{
				Photon* node = &mPhotons[i];
				for (int j = 0; j < Spectrum::SAMPLING_COUNT; ++j)
				{
					node->Power[j] *= scale;
				}
			}
			mPreviousScaleIndex = mStoredPhotons + 1;
		}

		void PhotonMap::balanceTree()
		{
			mHalfStoredPhotons = mStoredPhotons / 2 - 1;

			if(mStoredPhotons < 1)
			{
				return;
			}

			Photon** pTmp1 = new Photon*[mStoredPhotons + 1];
			Photon** pTmp2 = new Photon*[mStoredPhotons + 1];

			PR_ASSERT(pTmp1);
			PR_ASSERT(pTmp2);

			for (uint64 i = 0; i <= mStoredPhotons; ++i)
			{
				pTmp2[i] = &mPhotons[i];
			}

			balanceSegment(pTmp1, pTmp2, 1, 1, mStoredPhotons);
			delete[] pTmp2;
			pTmp2 = nullptr;// For debug!

			//Now reorganize balanced kd-tree to make use of the heap design
			uint64 d, j = 1, h = 1;
			Photon h_photon = mPhotons[j];

			for (uint64 i = 0; i <= mStoredPhotons; ++i)
			{
				d = pTmp1[j] - mPhotons;
				pTmp1[j] = nullptr;

				if (d == h)
				{
					mPhotons[j] = mPhotons[d];
					j = d;
				}
				else
				{
					mPhotons[j] = h_photon;

					if (i < mStoredPhotons)// Not the last entry
					{
						for (; h <= mStoredPhotons; ++h)
						{
							if (pTmp1[h])
							{
								break;
							}
						}

						h_photon = mPhotons[h];
						j = h;
					}
				}
			}

			delete[] pTmp1;
		}

		void PhotonMap::balanceSegment(Photon** balance, Photon** original, uint64 index, uint64 start, uint64 end)
		{
			// Calculate median
			uint64 median = 1;
			while ((4 * median) <= (end - start + 1))
				median += median;

			if ((3 * median) <= (end - start + 1))
				median += median + start - 1;
			else
				median = end - median + 1;

			// Find axis
			int axis = 0;// X axis
			if (mBox.height() > mBox.width() &&
				mBox.height() > mBox.depth())
				axis = 1;// Y axis
			else if (mBox.depth() > mBox.width())
				axis = 2;// Z axis

			// Partition
			medianSplit(original, start, end, median, axis);
			balance[index] = original[median];
			balance[index]->KDFlags = axis;

			// Recursively do the left and right part
			if (median > start)
			{
				if (start < median - 1)
				{
					float tmp = PM::pm_GetIndex(mBox.upperBound(), axis);
					mBox.setUpperBound(PM::pm_SetIndex(mBox.upperBound(), axis, balance[index]->Position[axis]));
					balanceSegment(balance, original, 2 * index, start, median - 1);
					mBox.setUpperBound(PM::pm_SetIndex(mBox.upperBound(), axis, tmp));
				}
				else
				{
					balance[2 * index] = original[start];
				}
			}

			if (median < end)
			{
				if (median + 1 < end)
				{
					float tmp = PM::pm_GetIndex(mBox.lowerBound(), axis);
					mBox.setLowerBound(PM::pm_SetIndex(mBox.lowerBound(), axis, balance[index]->Position[axis]));
					balanceSegment(balance, original, 2 * index + 1, median + 1, end);
					mBox.setLowerBound(PM::pm_SetIndex(mBox.lowerBound(), axis, tmp));
				}
				else
				{
					balance[2 * index + 1] = original[end];
				}
			}
		}

		void PhotonMap::medianSplit(Photon** photon, uint64 start, uint64 end, uint64 median, int axis)
		{
			uint64 left = start;
			uint64 right = end;

			while (right > left)
			{
				float v = photon[right]->Position[axis];
				uint64 i = left - 1;
				uint64 j = right;
				while (true)
				{
					while (photon[++i]->Position[axis] < v)
						;
					while (photon[--j]->Position[axis] > v && j > left)
						;

					if (i >= j)
						break;

					Photon* tmp = photon[i]; photon[i] = photon[j]; photon[j] = tmp;
				}
				Photon* tmp = photon[i]; photon[i] = photon[right]; photon[right] = tmp;

				if (i >= median)
					right = i - 1;
				if (i <= median)
					left = i + 1;
			}
		}
	}
}
namespace PR
{
	namespace Photon
	{
		template<class T>
		PointMap<T>::PointMap(uint64 max_photons) :
			mPhotons(nullptr),
			mStoredPhotons(0), mHalfStoredPhotons(0), mMaxPhotons(max_photons),
			mPreviousScaleIndex(1)
		{
			mPhotons = new T[max_photons + 1];
			PR_ASSERT(mPhotons);

			// Caching
			for (int i = 0; i < 256; ++i)
			{
				float angle = i * (1.0f / 256)*PM_PI_F;

				PM::pm_SinCosT(angle, mSinTheta[i], mCosTheta[i]);
				PM::pm_SinCosT(2 * angle, mSinPhi[i], mCosPhi[i]);
			}
		}

		template<class T>
		PointMap<T>::~PointMap()
		{
			delete[] mPhotons;
		}

		template<class T>
		void PointMap<T>::reset()
		{
			mStoredPhotons = 0;
			mHalfStoredPhotons = 0;
			mPreviousScaleIndex = 1;
		}

		template<class T>
		void PointMap<T>::locateSphere(PointSphere<T>& sphere, uint64 index)
		{
			locate(sphere, index, [](const T* pht, const PointSphere<T>& sph, float& dist2)
			{
				PM::vec3 V = PM::pm_Subtract(PM::pm_Set(pht->Position[0], pht->Position[1], pht->Position[2]), sph.Center);
				dist2 = PM::pm_MagnitudeSqr3D(V);
				const float d = PM::pm_Dot3D(V, sph.Normal);
				const float r = sph.Distances2[0] * (1 - std::abs(d)) +
					sph.Distances2[0] * std::abs(d) * (1 - sph.SqueezeWeight);
				return dist2 <= r;
			});
		}
	
		template<class T>
		void PointMap<T>::locateDome(PointSphere<T>& sphere, uint64 index)
		{
			locate(sphere, index, [](const T* pht, const PointSphere<T>& sph, float& dist2)
			{
				PM::vec3 V = PM::pm_Subtract(PM::pm_Set(pht->Position[0], pht->Position[1], pht->Position[2]), sph.Center);
				dist2 = PM::pm_MagnitudeSqr3D(V);
				const float d = PM::pm_Dot3D(V, sph.Normal);
				const float r = sph.Distances2[0] * (1 - std::abs(d)) +
					sph.Distances2[0] * std::abs(d) * (1 - sph.SqueezeWeight);
				return d <= -PM_EPSILON && dist2 <= r;
			});
		}

		template<class T>
		void PointMap<T>::locate(PointSphere<T>& sphere, uint64 index, CheckFunction checkFunc)
		{
			T* photon = &mPhotons[index];

			if (index < mHalfStoredPhotons)
			{
				float dist1 = PM::pm_GetIndex(sphere.Center, photon->KDFlags) - photon->Position[photon->KDFlags];
				if (dist1 > 0) // Right search
				{
					locate(sphere, 2 * index + 1, checkFunc);

					if (dist1 * dist1 < sphere.Distances2[0])
						locate(sphere, 2 * index, checkFunc);
				}
				else // Left search
				{
					locate(sphere, 2 * index, checkFunc);

					if (dist1 * dist1 < sphere.Distances2[0])
						locate(sphere, 2 * index + 1, checkFunc);
				}
			}

			// compute distance
			float dist2;
			if (checkFunc(photon, sphere, dist2))// Found a photon!
			{
				if (sphere.Found < sphere.Max)
				{
					sphere.Found++;
					sphere.Distances2[sphere.Found] = dist2;
					sphere.Index[sphere.Found] = photon;
				}
				else
				{
					uint64 j, parent;

					if (!sphere.GotHeap)
					{
						const T* photon2;
						uint64 halfFound = sphere.Found >> 1;
						for (uint64 k = halfFound; k >= 1; --k)
						{
							parent = k;
							photon2 = sphere.Index[k];
							float distTmp = sphere.Distances2[k];

							while (parent <= halfFound)
							{
								j = parent * 2;

								if (j < sphere.Found && sphere.Distances2[j] < sphere.Distances2[j + 1])
									j++;

								if (distTmp >= sphere.Distances2[j])
									break;

								sphere.Distances2[parent] = sphere.Distances2[j];
								sphere.Index[parent] = sphere.Index[j];
								parent = j;
							}

							sphere.Distances2[parent] = distTmp;
							sphere.Index[parent] = photon2;
						}
						sphere.GotHeap = true;
					}

					parent = 1;
					j = 2;
					while (j <= sphere.Found)
					{
						if (j < sphere.Found && sphere.Distances2[j] < sphere.Distances2[j + 1])
							j++;

						if (dist2 > sphere.Distances2[j])
							break;

						sphere.Distances2[parent] = sphere.Distances2[j];
						sphere.Index[parent] = sphere.Index[j];
						parent = j;
						j += j;
					}

					if (dist2 < sphere.Distances2[parent])
					{
						sphere.Distances2[parent] = dist2;
						sphere.Index[parent] = photon;
					}

					sphere.Distances2[0] = sphere.Distances2[1];
				}
			}
		}

		template<class T>
		void PointMap<T>::store(const PM::vec3& pos, const T& pht)
		{
			if (isFull())
				return;

			mStoredPhotons++;
			mPhotons[mStoredPhotons] = pht;

			mBox.put(pos);
		}

		template<class T>
		template<typename CallFunction>
		void PointMap<T>::callNewestPoints(CallFunction f)
		{
			for (uint64 i = mPreviousScaleIndex; i <= mStoredPhotons; ++i)
				f(&mPhotons[i]);
			mPreviousScaleIndex = mStoredPhotons + 1;
		}

		template<class T>
		void PointMap<T>::balanceTree()
		{
			if(mStoredPhotons > 1)
				mHalfStoredPhotons = mStoredPhotons / 2 - 1;
			else
				mHalfStoredPhotons = 0;

			if(mStoredPhotons < 1)
				return;

			T** pTmp1 = new T*[mStoredPhotons + 1];
			T** pTmp2 = new T*[mStoredPhotons + 1];

			PR_ASSERT(pTmp1);
			PR_ASSERT(pTmp2);

			for (uint64 i = 0; i <= mStoredPhotons; ++i)
				pTmp2[i] = &mPhotons[i];

			balanceSegment(pTmp1, pTmp2, 1, 1, mStoredPhotons);
			delete[] pTmp2;
			pTmp2 = nullptr;// For debug!

			//Now reorganize balanced kd-tree to make use of the heap design
			uint64 d, j = 1, h = 1;
			T h_photon = mPhotons[j];

			for (uint64 i = 1; i <= mStoredPhotons; ++i)
			{
				d = (uint64)(pTmp1[j] - mPhotons);
				pTmp1[j] = nullptr;

				if (d != h)
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

		template<class T>
		void PointMap<T>::balanceSegment(T** balance, T** original, uint64 index, uint64 start, uint64 end)
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
			int axis = 2;// Z axis
			if (mBox.width() > mBox.height() &&
				mBox.width() > mBox.depth())
				axis = 0;// X axis
			else if (mBox.height() > mBox.depth())
				axis = 1;// Y axis

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

		template<class T>
		void PointMap<T>::medianSplit(T** photon, uint64 start, uint64 end, uint64 median, int axis)
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

					T* tmp = photon[i]; photon[i] = photon[j]; photon[j] = tmp;
				}
				T* tmp = photon[i]; photon[i] = photon[right]; photon[right] = tmp;

				if (i >= median)
					right = i - 1;
				if (i <= median)
					left = i + 1;
			}
		}
	}
}
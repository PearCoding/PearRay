namespace PR
{
namespace Photon
{
	PointMap::PointMap(float gridDelta) :
		mPhotons(),
		mStoredPhotons(0), mGridDelta(gridDelta)
	{
		PR_ASSERT(mGridDelta > PM_EPSILON, "Grid delta has to greater 0");
		// Caching
		mInvGridDelta = 1.0f/mGridDelta;
		PR_ASSERT(std::isfinite(mInvGridDelta), "Inverse of grid delta has to be valid");

		for (int i = 0; i < 256; ++i)
		{
			float angle = i * (1.0f / 256)*PM_PI_F;

			PM::pm_SinCos(angle, mSinTheta[i], mCosTheta[i]);
			PM::pm_SinCos(2 * angle, mSinPhi[i], mCosPhi[i]);
		}
	}

	PointMap::~PointMap()
	{
	}

	void PointMap::reset()
	{
		mStoredPhotons = 0;
		mPhotons.clear();
	}

	template<typename AccumFunction>
	Spectrum PointMap::estimateSphere(const PointSphere& sphere, AccumFunction accumFunc, size_t& found) const
	{
		return estimate<AccumFunction>(sphere,
			[](const Photon& pht, const PointSphere& sph, float& dist2)
				{
					PM::vec3 V = PM::pm_Subtract(PM::pm_Set(pht.Position[0], pht.Position[1], pht.Position[2]), sph.Center);
					dist2 = PM::pm_MagnitudeSqr3D(V);
					const float d = PM::pm_Dot3D(V, sph.Normal);
					const float r = sph.Distance2 * (1 - std::abs(d)) +
						sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
					return dist2 <= r;
				},
			accumFunc, found);
	}

	template<typename AccumFunction>
	Spectrum PointMap::estimateDome(const PointSphere& sphere, AccumFunction accumFunc, size_t& found) const
	{
		return estimate<AccumFunction>(sphere,
			[](const Photon& pht, const PointSphere& sph, float& dist2)
				{
					PM::vec3 V = PM::pm_Subtract(PM::pm_Set(pht.Position[0], pht.Position[1], pht.Position[2]), sph.Center);
					dist2 = PM::pm_MagnitudeSqr3D(V);
					const float d = PM::pm_Dot3D(V, sph.Normal);
					const float r = sph.Distance2 * (1 - std::abs(d)) +
						sph.Distance2 * std::abs(d) * (1 - sph.SqueezeWeight);
					return d <= -PM_EPSILON && dist2 <= r;
				},
			accumFunc, found);
	}

	template<typename AccumFunction>
	Spectrum PointMap::estimate(const PointSphere& sphere,
		CheckFunction checkFunc, AccumFunction accumFunc, size_t& found) const
	{
		found = 0;

		KeyCoord centerCoord = toCoords(sphere.Center);
		const int32 rad = std::floor(std::sqrt(sphere.Distance2) * mInvGridDelta);
		const int32 rad2 = rad*rad;

		float dist2;
		Spectrum accum;

#ifdef PR_USE_APPROX_PHOTON_MAP
		for(int x = -rad; x <= rad; ++x)
		{
			for(int y = -rad; y <= rad; ++y)
			{
				for(int z = -rad; z <= rad; ++z)
				{
					if(x*x + y*y + z*z > rad2)
						continue;
					
					KeyCoord key = {
						centerCoord.X+x,
						centerCoord.Y+y,
						centerCoord.Z+z };

					typename Map::const_accessor acc;
					if(mPhotons.find(acc, key))
					{
						if (checkFunc((*acc).second.Approximation, sphere, dist2))// Found a photon!
						{
							found++;
							accum += accumFunc((*acc).second.Approximation, sphere, dist2);
						}
					}
				}
			}
		}
#else
		MinRadiusGenerator<3> generator(rad);
		while(generator && found < sphere.MaxPhotons)
		{
			const auto point = generator.next();
			if(point[0]*point[0] + point[1]*point[1] + point[2]*point[2] > rad2)
				continue;
			
			KeyCoord key = {
				centerCoord.X+point[0],
				centerCoord.Y+point[1],
				centerCoord.Z+point[2] };
			
			typename Map::const_accessor acc;
			if(mPhotons.find(acc, key))
			{
				for(const T& pht: (*acc).second)
				{
					if (checkFunc(pht, sphere, dist2))// Found a photon!
					{
						found++;
						accum += accumFunc(pht, sphere, dist2);
					}
				}
			}
		}
#endif

		return accum;
	}

	void PointMap::store(const PM::vec3& pos, const Photon& pht)
	{
		mStoredPhotons++;
		const auto key = toCoords(pos);

		typename Map::accessor acc;
		mPhotons.insert(acc, key);

#ifdef PR_USE_APPROX_PHOTON_MAP
		auto& approx = (*acc).second;
		float t = 1.0f/(approx.Count + 1.0f);

		approx.Approximation.Position[0] = pht.Position[0] * t + approx.Approximation.Position[0]*(1-t);
		approx.Approximation.Position[1] = pht.Position[1] * t + approx.Approximation.Position[1]*(1-t);
		approx.Approximation.Position[2] = pht.Position[2] * t + approx.Approximation.Position[2]*(1-t);
		approx.Approximation.Phi = pht.Phi * t + approx.Approximation.Phi*(1-t);
		approx.Approximation.Theta = pht.Theta * t + approx.Approximation.Theta*(1-t);
# if PR_PHOTON_RGB_MODE >= 1
		approx.Approximation.Power[0] = pht.Power[0] * t + approx.Approximation.Power[0]*(1-t);
		approx.Approximation.Power[1] = pht.Power[1] * t + approx.Approximation.Power[1]*(1-t);
		approx.Approximation.Power[2] = pht.Power[2] * t + approx.Approximation.Power[2]*(1-t);
# else
		approx.Approximation.Power = pht.Power * t + approx.Approximation.Power*(1-t);
# endif
		approx.Count += 1;
#else
		(*acc).second.push_back(pht);
#endif

		mBox.put(pos);
	}

	typename PointMap::KeyCoord PointMap::toCoords(const PM::vec3& v) const
	{
		PM::vec3 s = PM::pm_Scale(v, mInvGridDelta);
		return {
			(int32)std::floor(PM::pm_GetX(s)),
			(int32)std::floor(PM::pm_GetY(s)),
			(int32)std::floor(PM::pm_GetZ(s))
			};
	}

	bool PointMap::KeyCoord::operator ==(const KeyCoord& other) const
	{
		return X == other.X && Y == other.Y && Z == other.Z;
	}

	size_t PointMap::hash_compare::hash(const KeyCoord& coord)
	{
		constexpr size_t P1 = 863693;
		constexpr size_t P2 = 990383;
		constexpr size_t P3 = 729941;
		return ((size_t)coord.X * P1) ^
			((size_t)coord.Y * P2) ^
			((size_t)coord.Z * P3);
	}

	bool PointMap::hash_compare::equal(const KeyCoord& k1, const KeyCoord& k2)
	{
		return k1 == k2;
	}
}
}

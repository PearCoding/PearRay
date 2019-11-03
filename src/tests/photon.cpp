#include "Test.h"
#include "Random.h"
#include "photon/PhotonMap.h"
#include "spectral/SpectrumDescriptor.h"

using namespace PR;

PR_BEGIN_TESTCASE(Photon)
PR_TEST("Initialization")
{
	Photon::PhotonMap map(1.0f);

	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), 0ULL);
}
PR_TEST("Store")
{
	constexpr uint64 PHOTONS = 100;

	Photon::PhotonMap map(1.0f);
	Photon::Photon pht;
	pht.Position[0] = 0;
	pht.Position[1] = 1;
	pht.Position[2] = 2;

	for(uint64 k = 0; k < PHOTONS; ++k) {
		map.store(pht);
	}

	PR_CHECK_FALSE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), PHOTONS);

	map.reset();
	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), 0ULL);
}
PR_TEST("Search")
{
	constexpr uint64 PHOTONS = 100;

	Photon::PhotonMap map(0.1f);

	Photon::Photon pht;
	Random random(42);

	for(uint64 k = 0; k < PHOTONS; ++k) {
		pht.Position[0] = random.getFloat();
		pht.Position[1] = random.getFloat();
		pht.Position[2] = random.getFloat();
		map.store(pht);
	}

	Photon::PhotonSphere sphere;
	sphere.MaxPhotons = PHOTONS;
	sphere.Center = Vector3f(0,0,0);
	sphere.Distance2 = 4;

	auto emptyAccum = [&](Spectrum& accum, const Photon::Photon& photon, const Photon::PhotonSphere& sp, float d2){};
	PR::Spectrum spec(SpectrumDescriptor::createStandardSpectral());
	size_t found;

	map.estimateSphere(spec, sphere,
		emptyAccum,
		found);
	PR_CHECK_EQ(found, sphere.MaxPhotons);

	sphere.Center = Vector3f(5,0,0);
	map.estimateSphere(spec, sphere,
		emptyAccum,
		found);
	PR_CHECK_EQ(found, 0ULL);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Photon);
PRT_END_MAIN
#include "Random.h"
#include "Test.h"
#include "photon/PhotonMap.h"

using namespace PR;

PR_BEGIN_TESTCASE(Photon)
PR_TEST("Initialization")
{
	Photon::PhotonMap map(BoundingBox(1, 1, 1), 1.0f);

	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), 0ULL);
}
PR_TEST("Store")
{
	constexpr uint64 PHOTONS = 100;

	Photon::PhotonMap map(BoundingBox(5, 5, 5), 1.0f);
	Photon::Photon pht;
	pht.Position = Vector3f(0, 1, 2);

	for (uint64 k = 0; k < PHOTONS; ++k)
		map.store(pht);

	PR_CHECK_FALSE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), PHOTONS);

	map.reset();
	PR_CHECK_TRUE(map.isEmpty());
	PR_CHECK_EQ(map.storedPhotons(), 0ULL);
}
PR_TEST("Search")
{
	constexpr uint64 PHOTONS = 100;

	Photon::PhotonMap map(BoundingBox(2, 2, 2), 0.1f);

	Photon::Photon pht;
	Random random(42);

	for (uint64 k = 0; k < PHOTONS; ++k) {
		pht.Position[0] = random.getFloat();
		pht.Position[1] = random.getFloat();
		pht.Position[2] = random.getFloat();
		map.store(pht);
	}

	Photon::PhotonSphere sphere;
	sphere.Center	 = Vector3f(0, 0, 0);
	sphere.Distance2 = 4;

	auto emptyAccum = [&](SpectralBlob&, const Photon::Photon&, const Photon::PhotonSphere&, float) {};
	size_t found;

	map.estimateSphere(sphere, emptyAccum, found);
	PR_CHECK_EQ(found, PHOTONS);

	sphere.Center = Vector3f(5, 0, 0);
	map.estimateSphere(sphere, emptyAccum, found);
	PR_CHECK_EQ(found, 0ULL);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Photon);
PRT_END_MAIN
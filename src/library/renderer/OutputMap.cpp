#include "OutputMap.h"
#include "RenderContext.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"

#include "Diagnosis.h"

namespace PR {
OutputMap::OutputMap(RenderContext* renderer)
	: mRenderer(renderer)
	, mSpectral(new OutputSpectral(renderer))
{
}

OutputMap::~OutputMap()
{
	deinit();
}

void OutputMap::init()
{
	// Init outputs
	mSpectral->init();
	if (!mIntCounter[V_Samples])
		mIntCounter[V_Samples] = std::make_shared<OutputCounter>(mRenderer, 0);

	for (uint32 i = 0; i < V_1D_COUNT; ++i) {
		if (mInt1D[i])
			mInt1D[i]->init();
	}

	for (uint32 i = 0; i < V_COUNTER_COUNT; ++i) {
		if (mIntCounter[i])
			mIntCounter[i]->init();
	}

	for (uint32 i = 0; i < V_3D_COUNT; ++i) {
		if (mInt3D[i])
			mInt3D[i]->init();
	}

	for (const auto& p : mCustom1D)
		p.second->init();

	for (const auto& p : mCustomCounter)
		p.second->init();

	for (const auto& p : mCustom3D)
		p.second->init();

	for (const auto& p : mCustomSpectral)
		p.second->init();
}

void OutputMap::deinit()
{
	mSpectral->deinit();

	for (uint32 i = 0; i < V_1D_COUNT; ++i) {
		if (mInt1D[i]) {
			mInt1D[i]->deinit();
			mInt1D[i].reset();
		}
	}

	for (uint32 i = 0; i < V_COUNTER_COUNT; ++i) {
		if (mIntCounter[i]) {
			mIntCounter[i]->deinit();
			mIntCounter[i].reset();
		}
	}

	for (uint32 i = 0; i < V_3D_COUNT; ++i) {
		if (mInt3D[i]) {
			mInt3D[i]->deinit();
			mInt3D[i].reset();
		}
	}

	for (const auto& p : mCustom1D)
		p.second->deinit();
	mCustom1D.clear();

	for (const auto& p : mCustomCounter)
		p.second->deinit();
	mCustomCounter.clear();

	for (const auto& p : mCustom3D)
		p.second->deinit();
	mCustom3D.clear();

	for (const auto& p : mCustomSpectral)
		p.second->deinit();
	mCustomSpectral.clear();
}

void OutputMap::clear()
{
	mSpectral->clear();

	for (uint32 i = 0; i < V_1D_COUNT; ++i) {
		if (mInt1D[i])
			mInt1D[i]->clear();
	}

	for (uint32 i = 0; i < V_COUNTER_COUNT; ++i) {
		if (mIntCounter[i])
			mIntCounter[i]->clear();
	}

	for (uint32 i = 0; i < V_3D_COUNT; ++i) {
		if (mInt3D[i])
			mInt3D[i]->clear();
	}

	for (const auto& p : mCustom1D)
		p.second->clear();

	for (const auto& p : mCustomCounter)
		p.second->clear();

	for (const auto& p : mCustom3D)
		p.second->clear();

	for (const auto& p : mCustomSpectral)
		p.second->clear();
}

void OutputMap::pushFragment(const Eigen::Vector2i& p, const Spectrum& s, const ShaderClosure& sc)
{
	uint32 oldSample = getSampleCount(p);
	float t			 = 1.0f / (oldSample + 1.0f);

	PR_CHECK_NEGATIVE(t, "OutputMap::pushFragment");

	// Spectral
	Spectrum oldSpec = mSpectral->getFragmentBounded(p);
	Spectrum newSpec = oldSpec * (1 - t) + s * t;
	PR_CHECK_NEGATIVE(newSpec, "OutputMap::pushFragment");

	mSpectral->setFragmentBounded(p, newSpec);
	setSampleCount(p, oldSample + 1);

	// 3D
	if (mInt3D[V_Position])
		mInt3D[V_Position]->setFragmentBounded(p,
											   mInt3D[V_Position]->getFragmentBounded(p) * (1 - t) + sc.P * t);
	if (mInt3D[V_Normal])
		mInt3D[V_Normal]->setFragmentBounded(p,
											 mInt3D[V_Normal]->getFragmentBounded(p) * (1 - t) + sc.N * t);
	if (mInt3D[V_NormalG])
		mInt3D[V_NormalG]->setFragmentBounded(p,
											  mInt3D[V_NormalG]->getFragmentBounded(p) * (1 - t) + sc.Ng * t);
	if (mInt3D[V_Tangent])
		mInt3D[V_Tangent]->setFragmentBounded(p,
											  mInt3D[V_Tangent]->getFragmentBounded(p) * (1 - t) + sc.Nx * t);
	if (mInt3D[V_Bitangent])
		mInt3D[V_Bitangent]->setFragmentBounded(p,
												mInt3D[V_Bitangent]->getFragmentBounded(p) * (1 - t) + sc.Ny * t);
	if (mInt3D[V_View])
		mInt3D[V_View]->setFragmentBounded(p,
										   mInt3D[V_View]->getFragmentBounded(p) * (1 - t) + sc.V * t);
	if (mInt3D[V_UVW])
		mInt3D[V_UVW]->setFragmentBounded(p,
										  mInt3D[V_UVW]->getFragmentBounded(p) * (1 - t) + sc.UVW * t);
	if (mInt3D[V_DPDU])
		mInt3D[V_DPDU]->setFragmentBounded(p,
										   mInt3D[V_DPDU]->getFragmentBounded(p) * (1 - t) + sc.dPdU * t);
	if (mInt3D[V_DPDV])
		mInt3D[V_DPDV]->setFragmentBounded(p,
										   mInt3D[V_DPDV]->getFragmentBounded(p) * (1 - t) + sc.dPdV * t);
	if (mInt3D[V_DPDW])
		mInt3D[V_DPDW]->setFragmentBounded(p,
										   mInt3D[V_DPDW]->getFragmentBounded(p) * (1 - t) + sc.dPdW * t);
	if (mInt3D[V_DPDX])
		mInt3D[V_DPDX]->setFragmentBounded(p,
										   mInt3D[V_DPDX]->getFragmentBounded(p) * (1 - t) + sc.dPdX * t);
	if (mInt3D[V_DPDY])
		mInt3D[V_DPDY]->setFragmentBounded(p,
										   mInt3D[V_DPDY]->getFragmentBounded(p) * (1 - t) + sc.dPdY * t);
	if (mInt3D[V_DPDZ])
		mInt3D[V_DPDZ]->setFragmentBounded(p,
										   mInt3D[V_DPDZ]->getFragmentBounded(p) * (1 - t) + sc.dPdZ * t);
	if (mInt3D[V_DPDT])
		mInt3D[V_DPDT]->setFragmentBounded(p,
										   mInt3D[V_DPDT]->getFragment(p) * (1 - t) + sc.dPdT * t);

	// 1D
	if (mInt1D[V_Depth])
		mInt1D[V_Depth]->setFragmentBounded(p,
											mInt1D[V_Depth]->getFragmentBounded(p) * (1 - t) + std::sqrt(sc.Depth2) * t);
	if (mInt1D[V_Time])
		mInt1D[V_Time]->setFragmentBounded(p,
										   mInt1D[V_Time]->getFragmentBounded(p) * (1 - t) + sc.T * t);
	if (mInt1D[V_Material])
		mInt1D[V_Material]->setFragmentBounded(p,
											   mInt1D[V_Material]->getFragmentBounded(p) * (1 - t) + (sc.Material ? sc.Material->id() : 0) * t);

	// Counter
	if (mIntCounter[V_ID])
		mIntCounter[V_ID]->setFragmentBounded(p, sc.EntityID);
}

bool OutputMap::isPixelFinished(const Eigen::Vector2i& p) const
{
	return mIntCounter[V_Samples]->getFragmentBounded(p) >= mRenderer->settings().maxCameraSampleCount();
}

uint64 OutputMap::finishedPixelCount() const
{
	const uint32 rw = mRenderer->width();
	const uint32 rh = mRenderer->height();

	uint64 pixelsFinished = 0;
	for (uint32 j = 0; j < rh; ++j) {
		for (uint32 i = 0; i < rw; ++i) {
			if (isPixelFinished(Eigen::Vector2i(i, j)))
				++pixelsFinished;
		}
	}

	return pixelsFinished;
}
}

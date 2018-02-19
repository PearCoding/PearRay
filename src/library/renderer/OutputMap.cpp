#include "OutputMap.h"
#include "RenderContext.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
OutputMap::OutputMap(RenderContext* renderer)
	: mRenderer(renderer)
	, mInitialized(false)
	, mSpectral(new FrameBufferFloat(renderer->spectrumDescriptor()->samples(), 0.0f))
{
}

OutputMap::~OutputMap()
{
	if (mInitialized)
		deinit();
}

void OutputMap::init()
{
	PR_ASSERT(!mInitialized, "OutputMap already initialized");

	// Init outputs
	mSpectral->init(mRenderer);
	if (!mIntCounter[V_Samples])
		mIntCounter[V_Samples] = std::make_shared<FrameBufferUInt64>(1, 0);

	for (uint32 i = 0; i < V_1D_COUNT; ++i) {
		if (mInt1D[i])
			mInt1D[i]->init(mRenderer);
	}

	for (uint32 i = 0; i < V_COUNTER_COUNT; ++i) {
		if (mIntCounter[i])
			mIntCounter[i]->init(mRenderer);
	}

	for (uint32 i = 0; i < V_3D_COUNT; ++i) {
		if (mInt3D[i])
			mInt3D[i]->init(mRenderer);
	}

	for (auto p : mCustom1D)
		p.second->init(mRenderer);

	for (auto p : mCustomCounter)
		p.second->init(mRenderer);

	for (auto p : mCustom3D)
		p.second->init(mRenderer);

	for (auto p : mCustomSpectral)
		p.second->init(mRenderer);

	mInitialized = true;

	clear();
}

void OutputMap::deinit()
{
	PR_ASSERT(mInitialized, "OutputMap not initialized");

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

	for (auto p : mCustom1D)
		p.second->deinit();
	mCustom1D.clear();

	for (auto p : mCustomCounter)
		p.second->deinit();
	mCustomCounter.clear();

	for (auto p : mCustom3D)
		p.second->deinit();
	mCustom3D.clear();

	for (auto p : mCustomSpectral)
		p.second->deinit();
	mCustomSpectral.clear();

	mInitialized = false;
}

void OutputMap::clear()
{
	PR_ASSERT(mInitialized, "OutputMap not initialized");

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

	for (auto p : mCustom1D)
		p.second->clear();

	for (auto p : mCustomCounter)
		p.second->clear();

	for (auto p : mCustom3D)
		p.second->clear();

	for (auto p : mCustomSpectral)
		p.second->clear();
}

void OutputMap::pushFragment(const Eigen::Vector2i& p, const Spectrum& s, const ShaderClosure& sc)
{
	PR_ASSERT(mInitialized, "OutputMap not initialized");

	if(p(0) < 0 || p(0) >= mSpectral->width() ||
		p(1) < 0 || p(1) >= mSpectral->height())
		return;// Assert??

	uint32 oldSample = getSampleCount(p);
	float t			 = 1.0f / (oldSample + 1.0f);

	// Spectral
	float* spec = &mSpectral->getFragmentBounded(p, 0) + s.spectralStart();
	for (uint32 i = 0; i < s.samples(); ++i) {
		spec[i] = spec[i] * (1 - t) + s.value(i) * t;
	}
	setSampleCount(p, oldSample + 1);

	// 3D
	if (mInt3D[V_Position])
		setValue(V_Position, p, t, sc.P);
	if (mInt3D[V_Normal])
		setValue(V_Normal, p, t, sc.N);
	if (mInt3D[V_NormalG])
		setValue(V_NormalG, p, t, sc.Ng);
	if (mInt3D[V_Tangent])
		setValue(V_Tangent, p, t, sc.Nx);
	if (mInt3D[V_Bitangent])
		setValue(V_Bitangent, p, t, sc.Ny);
	if (mInt3D[V_View])
		setValue(V_View, p, t, sc.V);
	if (mInt3D[V_UVW])
		setValue(V_UVW, p, t, sc.UVW);
	if (mInt3D[V_DPDU])
		setValue(V_DPDU, p, t, sc.dPdU);
	if (mInt3D[V_DPDV])
		setValue(V_DPDV, p, t, sc.dPdV);
	if (mInt3D[V_DPDW])
		setValue(V_DPDW, p, t, sc.dPdW);
	if (mInt3D[V_DPDT])
		setValue(V_DPDT, p, t, sc.dPdT);

	// 1D
	if (mInt1D[V_Depth])
		mInt1D[V_Depth]->setFragmentBounded(p, 0,
											mInt1D[V_Depth]->getFragmentBounded(p) * (1 - t) + std::sqrt(sc.Depth2) * t);
	if (mInt1D[V_Time])
		mInt1D[V_Time]->setFragmentBounded(p, 0,
										   mInt1D[V_Time]->getFragmentBounded(p) * (1 - t) + static_cast<float>(sc.T) * t);
	if (mInt1D[V_Material])
		mInt1D[V_Material]->setFragmentBounded(p, 0,
											   mInt1D[V_Material]->getFragmentBounded(p) * (1 - t) + (sc.Material ? sc.Material->id() : 0) * t);

	// Counter
	if (mIntCounter[V_ID])
		mIntCounter[V_ID]->setFragmentBounded(p, 0, sc.EntityID);
}

// TODO: To heavy cost with new allocation insided Spectrum
const Spectrum OutputMap::getFragment(const Eigen::Vector2i& p) const
{
	return Spectrum(mRenderer->spectrumDescriptor(), 0, mSpectral->channels(), &mSpectral->getFragmentBounded(p));
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

void OutputMap::setValue(Variable3D var, const Eigen::Vector2i& p, float t, const Eigen::Vector3f& val)
{
	const Eigen::Vector3f intp = val * t;

	for (uint32 i = 0; i < 3; ++i) {
		mInt3D[var]->setFragmentBounded(p, i, mInt3D[var]->getFragmentBounded(p, i) * (1 - t) + intp(i));
	}
}

} // namespace PR

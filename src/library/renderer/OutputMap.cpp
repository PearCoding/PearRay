#include "OutputMap.h"
#include "RenderContext.h"
#include "RenderManager.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
OutputMap::OutputMap(RenderContext* renderer)
	: mRenderer(renderer)
	, mInitialized(false)
	, mSpectral(new FrameBufferFloat(
		  renderer->width(),
		  renderer->height(),
		  renderer->renderManager()->spectrumDescriptor()->samples(),
		  0.0f))
{
	mIntCounter[V_Samples] = std::make_shared<FrameBufferUInt32>(
		renderer->width(),
		renderer->height(),
		1,
		0);
}

OutputMap::~OutputMap()
{
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

	for (auto p : mCustom1D)
		p.second->clear();

	for (auto p : mCustomCounter)
		p.second->clear();

	for (auto p : mCustom3D)
		p.second->clear();

	for (auto p : mCustomSpectral)
		p.second->clear();
}

void OutputMap::pushFragment(const vuint32& pixelIndex, const ShadingPoint& s)
{
	vfloat oldSample = simdpp::to_float32(getSampleCount(pixelIndex));
	vfloat t		 = 1.0f / (oldSample + 1.0f);

	// Spectral
	mSpectral->blendFragment(pixelIndex, s.WavelengthIndex, s.Radiance, t);

	// TODO:

	// Increase sample count
	incSampleCount(pixelIndex);
}

} // namespace PR

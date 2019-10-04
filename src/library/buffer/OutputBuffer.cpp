#include "OutputBuffer.h"
#include "renderer/RenderContext.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

namespace PR {
OutputBuffer::OutputBuffer(RenderContext* renderer)
	: mRenderer(renderer)
	, mSpectral(new FrameBufferFloat(
		  renderer->spectrumDescriptor()->samples(),
		  renderer->width(),
		  renderer->height(),
		  0.0f))
{
	mIntCounter[V_Samples] = std::make_shared<FrameBufferUInt32>(
		renderer->spectrumDescriptor()->samples(),
		renderer->width(),
		renderer->height(),
		0);
}

OutputBuffer::~OutputBuffer()
{
}

void OutputBuffer::clear()
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

#define _3D_S(ch, v)                \
	if (mInt3D[ch])                 \
		for (int i = 0; i < 3; ++i) \
	mInt3D[ch]->blendFragment(pixelIndex, channel * 3 + i, v[i], t)

#define _1D_S(ch, v) \
	if (mInt1D[ch])  \
	mInt1D[ch]->blendFragment(pixelIndex, channel, v, t)

#define _C_S(ch, v)      \
	if (mIntCounter[ch]) \
	mIntCounter[ch]->blendFragment(pixelIndex, i, v, t)
void OutputBuffer::pushFragment(uint32 pixelIndex, const ShadingPoint& s)
{
	uint32 channel  = s.Ray.WavelengthIndex;
	float oldSample = getSampleCount(pixelIndex, channel);
	float t			= 1.0f / (oldSample + 1.0f);

	// Spectral
	mSpectral->blendFragment(pixelIndex, channel, s.Radiance, t);

	/*_3D_S(V_Position, s.Geometry.P);
	_3D_S(V_Normal, s.Ns);
	_3D_S(V_NormalG, s.Geometry.Ng);
	_3D_S(V_Tangent, s.Nx);
	_3D_S(V_Bitangent, s.Ny);
	_3D_S(V_View, s.Ray.Direction);
	_3D_S(V_UVW, s.Geometry.UVW);
	_3D_S(V_DPDT, s.Geometry.dPdT);

	_1D_S(V_Depth, s.Depth2);
	_1D_S(V_Time, s.Ray.Time);
	_1D_S(V_Material, s.Geometry.MaterialID);*/

	// Increase sample count
	incSampleCount(pixelIndex, channel);
}

} // namespace PR

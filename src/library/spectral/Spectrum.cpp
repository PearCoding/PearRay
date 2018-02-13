#include "Spectrum.h"
#include "SpectrumDescriptor.h"

namespace PR {
// Internal
Spectrum::Spectrum_Internal::Spectrum_Internal(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float* data)
	: Descriptor(descriptor)
	, Start(start)
	, End(end)
	, External(true)
	, Data(data)
{
}

Spectrum::Spectrum_Internal::Spectrum_Internal(const SpectrumDescriptor* descriptor, uint32 start, uint32 end)
	: Descriptor(descriptor)
	, Start(start)
	, End(end)
	, External(false)
	, Data(nullptr)
{
	Data = new float[end - start];
}

Spectrum::Spectrum_Internal::~Spectrum_Internal()
{
	if (External && Data) {
		delete[] Data;
	}
}

// Public
Spectrum::Spectrum(const SpectrumDescriptor* descriptor)
	: Spectrum(descriptor, 0, descriptor->samples())
{
}

Spectrum::Spectrum(const SpectrumDescriptor* descriptor, float initial)
	: Spectrum(descriptor, 0, descriptor->samples(), initial)
{
}

Spectrum::Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end)
	: mInternal(std::make_shared<Spectrum_Internal>(descriptor, start, end))
{
}

Spectrum::Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float initial)
	: Spectrum(descriptor, start, end)
{
	fill(initial);
}

Spectrum::Spectrum(const SpectrumDescriptor* descriptor, uint32 start, uint32 end, float* data)
	: mInternal(std::make_shared<Spectrum_Internal>(descriptor, start, end, data))
{
}

Spectrum Spectrum::clone() const
{
	Spectrum spec(descriptor(), spectralStart(), spectralEnd());
	spec.copyFrom(c_ptr());
	return spec;
}

constexpr float CANDELA = 683.002f;
void Spectrum::weightPhotometric()
{
	for (uint32 i = 0; i < samples(); ++i)
		setValue(i, value(i) * descriptor()->luminousFactor(i + spectralStart()) * CANDELA);
}

float Spectrum::luminousFlux_nm() const
{
	float flux = 0;
	for (uint32 i = 0; i < samples(); ++i)
		flux += value(i) * descriptor()->luminousFactor(i + spectralStart()) * descriptor()->integralDelta(i + spectralStart());

	return flux * CANDELA;
}
}

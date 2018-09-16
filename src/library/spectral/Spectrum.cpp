#include "Spectrum.h"
#include "SpectrumDescriptor.h"

#include "math/SIMath.h"
#include "math/SIMathConstants.h"
#include "math/SIMathStd.h"

#include <type_traits>

namespace PR {
// Internal
Spectrum::Spectrum_Internal::Spectrum_Internal(const std::shared_ptr<SpectrumDescriptor>& descriptor, uint32 start, uint32 end, float* data)
	: Descriptor(descriptor)
	, Start(start)
	, End(end)
	, External(true)
	, Data(data)
{
}

Spectrum::Spectrum_Internal::Spectrum_Internal(const std::shared_ptr<SpectrumDescriptor>& descriptor, uint32 start, uint32 end)
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
	if (!External && Data) {
		delete[] Data;
	}
}

// Public
Spectrum::Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor)
	: Spectrum(descriptor, 0, descriptor->samples())
{
}

Spectrum::Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, float initial)
	: Spectrum(descriptor, 0, descriptor->samples(), initial)
{
}

Spectrum::Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, uint32 start, uint32 end)
	: mInternal(std::make_shared<Spectrum_Internal>(descriptor, start, end))
{
}

Spectrum::Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, uint32 start, uint32 end, float initial)
	: Spectrum(descriptor, start, end)
{
	fill(initial);
}

Spectrum::Spectrum(const std::shared_ptr<SpectrumDescriptor>& descriptor, uint32 start, uint32 end, float* data)
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

// Has to be in double!
template <typename T = long double>
inline SI::SpectralIrradianceWavelengthU<T, 0>
blackbody_eq(const SI::TemperatureU<T, 0>& temp, const SI::LengthU<T, 0>& lambda_nm)
{
	using namespace SI;

	const auto c  = SI::constants::c<T>();
	const auto h  = SI::constants::h<T>();
	const auto kb = SI::constants::kb<T>();

	const auto c1 = 2 * h * c * c;
	const auto c2 = h * c / kb;

	const auto lambda5 = lambda_nm * (lambda_nm * lambda_nm) * (lambda_nm * lambda_nm);
	const auto f	   = c2 / (lambda_nm * temp);

	return (c1 / lambda5) / SI::expm1(f);
}

Spectrum Spectrum::blackbody(const std::shared_ptr<SpectrumDescriptor>& desc, float temp)
{
	SI::TemperatureU<long double, 0> T(temp);
	Spectrum spec(desc);
	for (uint32 i = 0; i < desc->samples(); ++i) {
		long double lambda = desc->wavelength(i) * PR_NM_TO_M_F;
		spec.setValue(i, static_cast<float>(
							 (long double)blackbody_eq(
								 T,
								 SI::LengthU<long double, 0>(lambda))));
	}

	return spec;
}
}

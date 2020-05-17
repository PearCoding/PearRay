#include "DefaultSRGB.h"
#include "serialization/BufferSerializer.h"

#ifdef PR_OS_LINUX
extern const unsigned char srgb_coeff_data[];
//extern const unsigned char * const srgb_coeff_end;
extern const unsigned int srgb_coeff_size;
#endif

namespace PR {
std::shared_ptr<SpectralUpsampler> DefaultSRGB::loadSpectralUpsampler()
{
#ifdef PR_OS_LINUX
	BufferSerializer serializer;
	serializer.open(const_cast<uint8*>(srgb_coeff_data), (size_t)srgb_coeff_size, true);
	return std::make_shared<SpectralUpsampler>(serializer);
#else
	// TODO
	return nullptr;
#endif
}
} // namespace PR
#include "DefaultSRGB.h"
#include "Logger.h"
#include "serialization/MemorySerializer.h"

#if defined(PR_OS_LINUX)
extern const unsigned char srgb_coeff_data[];
//extern const unsigned char * const srgb_coeff_end;
extern const unsigned int srgb_coeff_size;
#elif defined(PR_OS_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

static HRSRC hResource = nullptr;

static std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return "No Error";

	LPSTR messageBuffer = nullptr;
	size_t size			= ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								   NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	::LocalFree(messageBuffer);

	return message;
}
#endif

namespace PR {
std::shared_ptr<SpectralUpsampler> DefaultSRGB::loadSpectralUpsampler()
{
#if defined(PR_OS_LINUX)
	MemorySerializer serializer;
	serializer.open(const_cast<uint8*>(srgb_coeff_data), (size_t)srgb_coeff_size, true);
	return std::make_shared<SpectralUpsampler>(serializer);
#elif defined(PR_OS_WINDOWS)	
	if (!hResource)
		hResource = ::FindResource(nullptr, TEXT("SRGB_COEFF"), RT_RCDATA);

	if (!hResource) {
		PR_LOG(L_FATAL) << "Could not acquire internal srgb coeff resource: " << GetLastErrorAsString() << std::endl;
		return nullptr;
	}

	HGLOBAL mem = ::LoadResource(nullptr, hResource);
	if (!mem) {
		PR_LOG(L_FATAL) << "Could not load internal srgb coeff resource: " << GetLastErrorAsString() << std::endl;
		return nullptr;
	}

	size_t size = ::SizeofResource(nullptr, hResource);
	uint8* data = (uint8*)::LockResource(mem);
	if (!data) {
		PR_LOG(L_FATAL) << "Could not lock internal srgb coeff resource: " << GetLastErrorAsString() << std::endl;
		return nullptr;
	}

	MemorySerializer serializer;
	serializer.open(data, size, true);
	return std::make_shared<SpectralUpsampler>(serializer);
#else
	return nullptr;
#endif
}
} // namespace PR
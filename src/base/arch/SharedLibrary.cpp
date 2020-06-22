#include "SharedLibrary.h"

#ifdef PR_OS_LINUX
#include <dlfcn.h>
#elif PR_OS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#error Windows implementation missing
#endif

namespace PR {
struct SharedLibraryInternal {
#ifdef PR_OS_LINUX
	void* Handle;
#elif PR_OS_WINDOWS
	HINSTANCE Handle;
#endif

	SharedLibraryInternal(const std::string& path)
	{
#ifdef PR_OS_LINUX
		Handle = dlopen(path.c_str(), RTLD_LAZY);
		if (!Handle)
			throw std::runtime_error(dlerror());
#elif PR_OS_WINDOWS
		Handle = LoadLibraryA(path.c_str());
		if (!Handle) // TODO: Better use GetLastError()
			throw std::runtime_error("Could not load library");
#endif
	}

	~SharedLibraryInternal()
	{
#ifdef PR_OS_LINUX
		dlclose(Handle);
#elif PR_OS_WINDOWS
		FreeLibrary(Handle);
#endif
	}
};

SharedLibrary::SharedLibrary() {}

SharedLibrary::SharedLibrary(const std::wstring& file)
{
	const std::string u8 = std::string(file.begin(), file.end());

#ifdef PR_OS_LINUX
	try {
		mInternal.reset(new SharedLibraryInternal(u8 + ".so"));
	} catch (...) {
		mInternal.reset(new SharedLibraryInternal(u8));
	}
#elif PR_OS_WINDOWS
	try {
		mInternal.reset(new SharedLibraryInternal(u8 + ".dll"));
	} catch (...) {
		mInternal.reset(new SharedLibraryInternal(u8));
	}
#endif
}

SharedLibrary::~SharedLibrary() {}

void* SharedLibrary::symbol(const std::string& name) const
{
	if (!mInternal)
		return nullptr;

#ifdef PR_OS_LINUX
	return dlsym(mInternal->Handle, name.c_str());
#elif PR_OS_WINDOWS
	return GetProcAddress(mInternal->Handle, name.c_str());
#endif
}

void SharedLibrary::unload()
{
	mInternal.reset();
}
} // namespace PR
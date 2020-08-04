#include "Platform.h"

#ifdef PR_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace PR {
/*#ifdef PR_OS_WINDOWS
std::wstring encodePath(const std::wstring& path)
{
	return path;
}

std::wstring encodePath(const std::string& path)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), (int)path.length(), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), (int)path.length(), &ret[0], len);
	}
	return ret;
}
#else
std::string encodePath(const std::wstring& path)
{
	return std::string(path.begin(), path.end());
}

std::string encodePath(const std::string& path)
{
	return path;
}
#endif*/
}
#include "CacheManager.h"
#include "Logger.h"

#include <boost/filesystem.hpp>
#include <cctype>

namespace PR {
CacheManager::CacheManager(const std::wstring& workingDir)
	: mWorkingDir(workingDir)
{
}

void CacheManager::clear()
{
	mFiles.clear();
}

std::wstring CacheManager::requestFile(const std::string& grp, const std::string& name,
									   bool& exists)
{
	std::string dir = grp;
	std::transform(dir.begin(), dir.end(), dir.begin(),
				   [](char c) { return std::tolower(c); });

	boost::filesystem::path root = mWorkingDir;
	root						 = root / "cache" / dir;

	boost::system::error_code error_code;
	boost::filesystem::create_directories(root, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in cache manager: " << error_code.message();
		return L"";
	}

	root = root / name;

	std::wstring path = root.generic_wstring();
	exists			  = (mFiles.count(path) > 0);

	if (!exists)
		mFiles.insert(path);

	return path;
}
} // namespace PR

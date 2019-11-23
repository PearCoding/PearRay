#pragma once

#include "PR_Config.h"
#include <unordered_set>

namespace PR {
class PR_LIB_UTILS CacheManager {
public:
	CacheManager(const std::wstring& workingDir);

	void clear();
	std::wstring requestFile(const std::string& grp, const std::string& name, bool& exists);
	inline std::wstring requestFile(const std::string& grp, const std::string& name)
	{
		bool _ignore;
		return requestFile(grp, name, _ignore);
	}

private:
	std::wstring mWorkingDir;
	std::unordered_set<std::wstring> mFiles;
};
} // namespace PR

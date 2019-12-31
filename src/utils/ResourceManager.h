#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_UTILS ResourceManager {
public:
	ResourceManager(const std::wstring& workingDir);

	std::wstring requestFile(const std::string& grp, const std::string& name, const std::wstring& dependency, bool& updateNeeded);
	std::wstring requestFile(const std::string& grp, const std::string& name, bool& updateNeeded);
	inline std::wstring requestFile(const std::string& grp, const std::string& name)
	{
		bool _ignore;
		return requestFile(grp, name, _ignore);
	}

private:
	std::wstring mWorkingDir;
};
} // namespace PR

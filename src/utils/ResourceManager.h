#pragma once

#include "PR_Config.h"
#include <unordered_map>

namespace PR {
class PR_LIB_UTILS ResourceManager {
public:
	ResourceManager(const std::wstring& workingDir);

	std::wstring requestFile(const std::string& grp, const std::string& name, const std::string& ext, bool& updateNeeded);
	inline std::wstring requestFile(const std::string& grp, const std::string& name, const std::string& ext)
	{
		bool _ignore;
		return requestFile(grp, name, ext, _ignore);
	}

	void addDependency(const std::string& grp, const std::string& name, const std::wstring& path);

private:
	size_t generateID(const std::string& grp, const std::string& name) const;
	bool checkDependencies(size_t id, const std::wstring& req_file) const;

	std::wstring mWorkingDir;

	std::unordered_multimap<size_t, std::wstring> mRequestedFiles;
	std::unordered_multimap<size_t, std::wstring> mDependencies;
};
} // namespace PR

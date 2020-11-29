#pragma once

#include "PR_Config.h"

#include <filesystem>
#include <unordered_map>

namespace PR {
class PR_LIB_LOADER ResourceManager {
public:
	ResourceManager(const std::filesystem::path& workingDir);

	std::filesystem::path requestFile(const std::string& grp, const std::string& name, const std::string& ext, bool& updateNeeded);
	inline std::filesystem::path requestFile(const std::string& grp, const std::string& name, const std::string& ext)
	{
		bool _ignore;
		return requestFile(grp, name, ext, _ignore);
	}

	void addDependency(const std::string& grp, const std::string& name, const std::filesystem::path& path);

private:
	size_t generateID(const std::string& grp, const std::string& name) const;
	bool checkDependencies(size_t id, const std::filesystem::path& req_file) const;

	const std::filesystem::path mWorkingDir;
	const bool mQueryMode;
 
	std::unordered_multimap<size_t, std::filesystem::path> mRequestedFiles;
	std::unordered_multimap<size_t, std::filesystem::path> mDependencies;
};
} // namespace PR

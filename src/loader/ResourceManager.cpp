#include "ResourceManager.h"
#include "Logger.h"

#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <cctype>

namespace PR {
ResourceManager::ResourceManager(const std::wstring& workingDir)
	: mWorkingDir(workingDir)
{
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "mesh");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "scene");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "node");
}

std::wstring ResourceManager::requestFile(const std::string& grp, const std::string& name, const std::string& ext,
										  bool& updateNeeded)
{
	std::string dir = grp;
	std::transform(dir.begin(), dir.end(), dir.begin(),
				   [](char c) { return std::tolower(c); });

	boost::filesystem::path root = mWorkingDir;
	root						 = root / "cache" / dir;

	boost::system::error_code error_code;
	boost::filesystem::create_directories(root, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager [create_directories]: " << error_code.message() << std::endl;
		return L"";
	}

	root = root / (name + ext);

	std::wstring path = root.generic_wstring();
	updateNeeded	  = !boost::filesystem::exists(root, error_code);
	if (error_code) {
		updateNeeded = true;
		//PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;
		//return L"";
	}

	size_t id = generateID(grp, name); // Without extension!
	if (!updateNeeded)
		updateNeeded = checkDependencies(id, path);

	mRequestedFiles.emplace(std::make_pair(id, path));

	return path;
}

size_t ResourceManager::generateID(const std::string& grp, const std::string& name) const
{
	size_t id;
	boost::hash_combine(id, grp);
	boost::hash_combine(id, name);
	return id;
}

void ResourceManager::addDependency(const std::string& grp, const std::string& name, const std::wstring& path)
{
	boost::system::error_code error_code;
	if (!boost::filesystem::exists(path, error_code))
		PR_LOG(L_WARNING) << "Given dependency " << boost::filesystem::path(path) << " does not even exists!" << std::endl;

	if (error_code)
		PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;

	size_t id = generateID(grp, name);
	mDependencies.emplace(std::make_pair(id, path));
}

bool ResourceManager::checkDependencies(size_t id, const std::wstring& req_file) const
{
	auto range = mDependencies.equal_range(id);
	for (auto it = range.first; it != range.second; ++it) {
		std::wstring dependency = it->second;
		boost::system::error_code error_code;
		if (!boost::filesystem::exists(dependency, error_code))
			return true;

		if (error_code) {
			return true;
			/*PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;
			continue;*/
		}

		std::time_t dep_time = boost::filesystem::last_write_time(dependency, error_code);
		if (error_code) {
			PR_LOG(L_ERROR) << "Error in resource manager [last_write_time]: " << error_code.message() << std::endl;
			continue;
		}

		std::time_t file_time = boost::filesystem::last_write_time(req_file, error_code);
		if (error_code) {
			PR_LOG(L_ERROR) << "Error in resource manager [last_write_time]: " << error_code.message() << std::endl;
			continue;
		}

		if (dep_time > file_time)
			return true;
	}

	return range.first == range.second; // True if no dependency
}
} // namespace PR

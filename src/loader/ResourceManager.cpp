#include "ResourceManager.h"
#include "Logger.h"
#include "math/Hash.h"

#include <cctype>

namespace PR {

ResourceManager::ResourceManager(const std::filesystem::path& workingDir)
	: mWorkingDir(workingDir)
	, mQueryMode(workingDir.empty())
{
	if (!mQueryMode) {
		std::filesystem::create_directories(workingDir / "cache" / "mesh");
		std::filesystem::create_directories(workingDir / "cache" / "scene");
		std::filesystem::create_directories(workingDir / "cache" / "node");
	}
}

// TODO: Handle query mode properly
std::filesystem::path ResourceManager::requestFile(const std::string& grp, const std::string& name, const std::string& ext,
												   bool& updateNeeded)
{
	if (mQueryMode)
		return {};

	std::string dir = grp;
	std::transform(dir.begin(), dir.end(), dir.begin(),
				   [](char c) { return std::tolower(c); });

	std::filesystem::path root = mWorkingDir;
	root					   = root / "cache" / dir;

	std::error_code error_code;
	std::filesystem::create_directories(root, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager [create_directories]: " << error_code.message() << std::endl;
		return L"";
	}

	root = root / (name + ext);

	updateNeeded = !std::filesystem::exists(root, error_code);
	if (error_code) {
		updateNeeded = true;
		//PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;
		//return L"";
	}

	size_t id = generateID(grp, name); // Without extension!
	if (!updateNeeded)
		updateNeeded = checkDependencies(id, root);

	mRequestedFiles.emplace(std::make_pair(id, root));

	return root;
}

size_t ResourceManager::generateID(const std::string& grp, const std::string& name) const
{
	size_t id = 0;
	hash_combine(id, grp);
	hash_combine(id, name);
	return id;
}

void ResourceManager::addDependency(const std::string& grp, const std::string& name, const std::filesystem::path& path)
{
	std::error_code error_code;
	if (!std::filesystem::exists(path, error_code))
		PR_LOG(L_WARNING) << "Given dependency " << path << " does not even exists!" << std::endl;

	if (error_code)
		PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;

	size_t id = generateID(grp, name);
	mDependencies.emplace(std::make_pair(id, path));
}

bool ResourceManager::checkDependencies(size_t id, const std::filesystem::path& req_file) const
{
	auto range = mDependencies.equal_range(id);
	for (auto it = range.first; it != range.second; ++it) {
		std::filesystem::path dependency = it->second;
		std::error_code error_code;
		if (!std::filesystem::exists(dependency, error_code))
			return true;

		if (error_code) {
			return true;
			/*PR_LOG(L_ERROR) << "Error in resource manager [exists]: " << error_code.message() << std::endl;
			continue;*/
		}

		auto dep_time = std::filesystem::last_write_time(dependency, error_code);
		if (error_code) {
			PR_LOG(L_ERROR) << "Error in resource manager [last_write_time]: " << error_code.message() << std::endl;
			continue;
		}

		auto file_time = std::filesystem::last_write_time(req_file, error_code);
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

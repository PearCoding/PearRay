#include "ResourceManager.h"
#include "Logger.h"

#include <boost/filesystem.hpp>
#include <cctype>

namespace PR {
ResourceManager::ResourceManager(const std::wstring& workingDir)
	: mWorkingDir(workingDir)
{
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "mesh");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "scene");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "tex");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "io");
	boost::filesystem::create_directories(boost::filesystem::path(workingDir) / "cache" / "node");
}

std::wstring ResourceManager::requestFile(const std::string& grp, const std::string& name,
										  const std::wstring& dependency,
										  bool& updateNeeded)
{
	std::wstring path = requestFile(grp, name, updateNeeded);
	if (path.empty() || updateNeeded)
		return path;

	boost::system::error_code error_code;
	if (!boost::filesystem::exists(dependency, error_code)) {
		updateNeeded = true;
		return path;
	}

	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager: " << error_code.message();
		return L"";
	}

	std::time_t dep_time = boost::filesystem::last_write_time(dependency, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager: " << error_code.message();
		return L"";
	}

	std::time_t file_time = boost::filesystem::last_write_time(path, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager: " << error_code.message();
		return L"";
	}

	updateNeeded = dep_time > file_time;
	return path;
}

std::wstring ResourceManager::requestFile(const std::string& grp, const std::string& name,
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
		PR_LOG(L_ERROR) << "Error in resource manager: " << error_code.message();
		return L"";
	}

	root = root / name;

	std::wstring path = root.generic_wstring();
	updateNeeded	  = !boost::filesystem::exists(root, error_code);
	if (error_code) {
		PR_LOG(L_ERROR) << "Error in resource manager: " << error_code.message();
		return L"";
	}

	return path;
}
} // namespace PR

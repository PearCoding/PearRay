#pragma once

#include "PR_Config.h"

namespace PR {
namespace Build {
struct PR_LIB_BASE Version {
	uint32 Major;
	uint32 Minor;
	inline uint32 asNumber() const { return ((Major) << 8) | (Minor); }
};
/**
 * @brief Returns version of the build
 * 
 * @return Version struct containing major and minor version 
 */
Version PR_LIB_BASE getVersion();
/**
 * @brief Returns version of the build as a string
 * 
 * @return std::string getVersionString 
 */
std::string PR_LIB_BASE getVersionString();
/**
 * @brief Returns git branch and revision of the build as a string
 * 
 * @return std::string getGitString 
 */
std::string PR_LIB_BASE getGitString();
/**
 * @brief Returns copyright of the build as a string
 * 
 * @return std::string getCopyrightString 
 */
std::string PR_LIB_BASE getCopyrightString();
/**
 * @brief Returns name of compiler used for compiling
 * 
 * @return std::string getCompilerName 
 */
std::string PR_LIB_BASE getCompilerName();
/**
 * @brief Returns name of operating system the software was compiled with
 * 
 * @return std::string getOSName 
 */
std::string PR_LIB_BASE getOSName();
/**
 * @brief Returns name of variant. Most of the time Debug or Release
 * 
 * @return std::string getBuildVariant 
 */
std::string PR_LIB_BASE getBuildVariant();
/**
 * @brief Returns list of features separated by ';'
 * 
 * @return std::string getFeatureSet()
 */
std::string PR_LIB_BASE getFeatureSet();
/**
 * @brief Composed string representing all important options used while compiling the software
 * 
 * @return std::string getBuildString 
 */
std::string PR_LIB_BASE getBuildString();
} // namespace Build
} // namespace PR
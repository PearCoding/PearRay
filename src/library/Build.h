#pragma once

#include "PR_Config.h"

namespace PR {
namespace Build {
/**
 * @brief Returns name of compiler used for compiling
 * 
 * @return std::string getCompilerName 
 */
std::string PR_LIB getCompilerName();
/**
 * @brief Returns name of operating system the software was compiled with
 * 
 * @return std::string getOSName 
 */
std::string PR_LIB getOSName();
/**
 * @brief Returns name of variant. Most of the time Debug or Release
 * 
 * @return std::string getBuildVariant 
 */
std::string PR_LIB getBuildVariant();
/**
 * @brief Returns best feature set of SIMD used inside the software
 * 
 * @return std::string getSIMDLevel 
 */
std::string PR_LIB getSIMDLevel();
/**
 * @brief Composed string representing all important options used while compiling the software
 * 
 * @return std::string getBuildString 
 */
std::string PR_LIB getBuildString();
} // namespace Build
} // namespace PR
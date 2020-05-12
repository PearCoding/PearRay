#include "LightPathManager.h"

#include <functional>

namespace PR {
size_t LightPathManager::getLabelIndex(const std::string& lbl)
{
	std::hash<std::string> hash;
	std::string tmp;
	std::transform(lbl.begin(), lbl.end(), tmp.begin(), ::tolower);
	return hash(tmp);
}
} // namespace PR
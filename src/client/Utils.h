#pragma once

#include "PR_Config.h"
#include <sstream>

namespace PR {
inline std::string timestr(uint64 sec)
{
	if (sec == 0)
		return " <1s";

	std::stringstream sstream;

	uint64 s = sec % 60;
	sec /= 60;
	uint64 m = sec % 60;
	sec /= 60;
	uint64 h = sec;

	if (h > 0)
		sstream << h << "h ";

	if (m > 0)
		sstream << m << "m ";

	if (s > 0)
		sstream << s << "s ";

	return sstream.str();
}
} // namespace PR
#pragma once

#include "PR_Config.h"
#include <chrono>

namespace PR {
using time_point_t = std::chrono::high_resolution_clock::time_point;

struct UpdateInfo {
	time_point_t Start;
	uint32 CurrentIteration;
	uint32 CurrentPass;
};

class ProgramSettings;
class RenderContext;
class IProgressObserver {
public:
	virtual ~IProgressObserver() = default;

	virtual void begin(RenderContext* renderContext, const ProgramSettings& settings) = 0;
	virtual void end()																  = 0;
	virtual void update(const UpdateInfo& info)										  = 0;
	virtual void onIteration(const UpdateInfo& info)								  = 0;
};
} // namespace PR
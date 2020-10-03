#pragma once

#include "IProgressObserver.h"
#include "spectral/ToneMapper.h"

namespace PR {
class Environment;
class StatusObserver : public IProgressObserver {
public:
	StatusObserver();
	virtual ~StatusObserver();

	void begin(RenderContext* renderContext, const ProgramSettings& settings) override;
	void end() override;
	void update(const UpdateInfo& info) override;
	void onIteration(const UpdateInfo& info) override;

private:
	RenderContext* mRenderContext;

	uint64 mUpdateCycleSeconds;

	time_point_t mLastUpdate;

	bool mBeautify;
	bool mFirstTime;
};
} // namespace PR
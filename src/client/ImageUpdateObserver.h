#pragma once

#include "IProgressObserver.h"
#include "spectral/ToneMapper.h"

namespace PR {
class Environment;
class ImageUpdateObserver : public IProgressObserver {
public:
	ImageUpdateObserver(Environment* environment);
	virtual ~ImageUpdateObserver();

	void begin(RenderContext* renderContext, const ProgramSettings& settings) override;
	void end() override;
	void update(const UpdateInfo& info) override;
	void onIteration(const UpdateInfo& info) override;

private:
	void save(const UpdateInfo& info);

	RenderContext* mRenderContext;
	Environment* mEnvironment;
	ToneMapper mToneMapper;

	uint32 mIterationCount;

	uint32 mIterationCycleCount;
	uint64 mUpdateCycleSeconds;

	time_point_t mLastUpdate;

	bool mUseTags;
};
} // namespace PR
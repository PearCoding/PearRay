#pragma once

#include "IProgressObserver.h"

namespace PR {
// Connect with https://github.com/Tom94/tev for active view
class TevObserver : public IProgressObserver {
public:
	TevObserver();
	virtual ~TevObserver();

	void begin(RenderContext* renderContext, const ProgramSettings& settings) override;
	void end() override;
	void update(const UpdateInfo& info) override;
	void onIteration(const UpdateInfo& info) override;

private:
	void createImageProtocol();
	void closeImageProtocol();
	void updateImageProtocol();

	std::unique_ptr<class TevConnection> mConnection;
	RenderContext* mRenderContext;
	uint64 mUpdateCycleSeconds;

	time_point_t mLastUpdate;
};
} // namespace PR
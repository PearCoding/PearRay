#pragma once

#include "IProgressObserver.h"

#include <atomic>

namespace PR {
class NetworkServer;
class NetworkObserver : public IProgressObserver {
public:
	NetworkObserver();
	virtual ~NetworkObserver();

	void begin(RenderContext* renderContext, const ProgramSettings& settings) override;
	void end() override;
	void update(const UpdateInfo& info) override;
	void onIteration(const UpdateInfo& info) override;

	inline uint32 currentIteration() const { return mIterationCount; }
	inline RenderContext* context() const { return mRenderContext; }

private:
	std::unique_ptr<NetworkServer> mServer;
	RenderContext* mRenderContext;

	std::atomic<uint32> mIterationCount;
};
} // namespace PR
#pragma once

#include "OutputData.h"
#include "OutputSystem.h"

namespace PR {
class LocalOutputDevice;
class StreamPipeline;
class RenderThread;

/// Per Thread/Tile based output system
class PR_LIB_CORE LocalOutputSystem {
	friend class OutputSystem;

public:
	explicit LocalOutputSystem(const RenderTile* tileRenderTile, const OutputSystem* parent, const Size2i& localSize);
	~LocalOutputSystem();

	inline const Size2i& globalSize() const { return mParent->size(); }
	inline const Size2i& localSize() const { return mLocalSize; }

	void clear(bool force = false);

	void commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entrycount);
	void commitShadingPoints(const OutputShadingPointEntry* entries, size_t entrycount);
	void commitFeedbacks(const OutputFeedbackEntry* entries, size_t entrycount);

	void commitCustomSpectrals(uint32 aov_id, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entrycount);
	void commitCustom3D(uint32 aov_id, const OutputCustom3DEntry* entries, size_t entrycount);
	void commitCustom1D(uint32 aov_id, const OutputCustom1DEntry* entries, size_t entrycount);
	void commitCustomCounter(uint32 aov_id, const OutputCustomCounterEntry* entries, size_t entrycount);

	void beforeMerge();
	
protected:
	void addLocalOutputDevice(const std::shared_ptr<LocalOutputDevice>& device);
	std::shared_ptr<LocalOutputDevice> localOutputDevice(size_t i) const;

private:
	const RenderTile* mTile;
	const OutputSystem* mParent;
	const Size2i mLocalSize;
	std::vector<std::shared_ptr<LocalOutputDevice>> mLocalOutputDevices;
};
} // namespace PR

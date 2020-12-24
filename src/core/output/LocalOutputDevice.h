#pragma once

#include "OutputData.h"

namespace PR {
class StreamPipeline;

/// Abstract local output device
/// The parent output device will create this local output device such that only one thread will access it
class PR_LIB_CORE LocalOutputDevice {
public:
	virtual void clear(bool force = false)																									 = 0;
	virtual void commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entrycount)							 = 0;
	virtual void commitShadingPoints(const OutputShadingPointEntry* entries, size_t entrycount)												 = 0;
	virtual void commitFeedbacks(const OutputFeedbackEntry* entries, size_t entrycount)														 = 0;
	virtual void commitCustomSpectrals(uint32 aov_id, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entrycount) = 0;
	virtual void commitCustom3D(uint32 aov_id, const OutputCustom3DEntry* entries, size_t entrycount)										 = 0;
	virtual void commitCustom1D(uint32 aov_id, const OutputCustom1DEntry* entries, size_t entrycount)										 = 0;
	virtual void commitCustomCounter(uint32 aov_id, const OutputCustomCounterEntry* entries, size_t entrycount)								 = 0;

	virtual void beforeMerge() {}
};
} // namespace PR
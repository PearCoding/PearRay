#pragma once

#include "OutputData.h"

namespace PR {
class RenderContext;

struct PR_LIB_CORE OutputCommitInformation {
	Point2i TileStart;
	uint32 ThreadID;
};

/// Abstract output device
class PR_LIB_CORE OutputDevice {
public:
	virtual void commitSpectrals(const OutputCommitInformation& info, const OutputSpectralEntry* entries, size_t entrycount)		 = 0;
	virtual void commitShadingPoints(const OutputCommitInformation& info, const OutputShadingPointEntry* entries, size_t entrycount) = 0;
	virtual void commitFeedbacks(const OutputCommitInformation& info, const OutputFeedbackEntry* entries, size_t entrycount)		 = 0;

	virtual void onStart(RenderContext* ctx) = 0;
	virtual void onNextIteration()			 = 0;
	virtual void onStop()					 = 0;
};
} // namespace PR
#pragma once

#include "OutputBufferData.h"
#include <mutex>

namespace PR {
class IFilter;
class OutputBufferBucket;
class PR_LIB OutputBuffer {
public:
	explicit OutputBuffer(const std::shared_ptr<IFilter>& filter,
						  const Size2i& size, Size1i specChannels);
	~OutputBuffer();

	inline OutputBufferData& data() { return mData; }
	inline const OutputBufferData& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	std::shared_ptr<OutputBufferBucket> createBucket(const Size2i& size) const;
	void mergeBucket(const Point2i& p, const std::shared_ptr<OutputBufferBucket>& bucket);

private:
	std::shared_ptr<IFilter> mFilter;
	OutputBufferData mData;
	std::mutex mMergeMutex;
};
} // namespace PR

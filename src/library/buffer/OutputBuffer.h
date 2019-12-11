#pragma once

#include "OutputBufferData.h"
#include <mutex>

namespace PR {
class IFilter;
class OutputBufferBucket;
class PR_LIB OutputBuffer {
public:
	explicit OutputBuffer(const std::shared_ptr<IFilter>& filter,
						  size_t width, size_t height, size_t specChannels);
	~OutputBuffer();

	inline OutputBufferData& data() { return mData; }
	inline const OutputBufferData& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	std::shared_ptr<OutputBufferBucket> createBucket(size_t width, size_t height) const;
	void mergeBucket(size_t ox, size_t oy, const std::shared_ptr<OutputBufferBucket>& bucket);

private:
	std::shared_ptr<IFilter> mFilter;
	OutputBufferData mData;
	std::mutex mMergeMutex;
};
} // namespace PR

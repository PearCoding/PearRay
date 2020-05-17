#pragma once

#include "FrameBufferContainer.h"
#include <mutex>

namespace PR {
class IFilter;
class FrameBufferBucket;
class PR_LIB_CORE FrameBufferSystem {
public:
	explicit FrameBufferSystem(const std::shared_ptr<IFilter>& filter,
						  const Size2i& size, Size1i specChannels);
	~FrameBufferSystem();

	inline FrameBufferContainer& data() { return mData; }
	inline const FrameBufferContainer& data() const { return mData; }

	inline void clear(bool force = false) { mData.clear(force); }

	std::shared_ptr<FrameBufferBucket> createBucket(const Size2i& size) const;
	void mergeBucket(const Point2i& p, const std::shared_ptr<FrameBufferBucket>& bucket);

private:
	std::shared_ptr<IFilter> mFilter;
	FrameBufferContainer mData;
	std::mutex mMergeMutex;
};
} // namespace PR

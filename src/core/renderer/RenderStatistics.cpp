#include "RenderStatistics.h"

namespace PR {
RenderStatistics::RenderStatistics()
{
	for (int i = 0; i < _RST_COUNT_; ++i)
		mCounters[i] = 0;
}

RenderStatistics::RenderStatistics(const RenderStatistics& other)
{
	for (int i = 0; i < _RST_COUNT_; ++i)
		mCounters[i] = other.mCounters[i].load();
}

RenderStatistics& RenderStatistics::operator=(const RenderStatistics& other)
{
	for (int i = 0; i < _RST_COUNT_; ++i)
		mCounters[i] = other.mCounters[i].load();
	return *this;
}

RenderStatistics& RenderStatistics::operator+=(const RenderStatistics& other)
{
	for (int i = 0; i < _RST_COUNT_; ++i)
		mCounters[i] += other.mCounters[i].load();
	return *this;
}

RenderStatistics RenderStatistics::half() const
{
	RenderStatistics other;
	for (int i = 0; i < _RST_COUNT_; ++i)
		other.mCounters[i] = mCounters[i].load() / 2;
	return other;
}
} // namespace PR

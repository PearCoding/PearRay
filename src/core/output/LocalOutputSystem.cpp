#include "LocalOutputSystem.h"
#include "LocalOutputDevice.h"

namespace PR {
LocalOutputSystem::LocalOutputSystem(const OutputSystem* parent, const Size2i& localSize)
	: mParent(parent)
	, mLocalSize(localSize)
{
}

LocalOutputSystem::~LocalOutputSystem()
{
}

void LocalOutputSystem::clear(bool force)
{
	for (const auto& device : mLocalOutputDevices)
		device->clear(force);
}

void LocalOutputSystem::commitSpectrals(StreamPipeline* pipeline, const OutputSpectralEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitSpectrals(pipeline, entries, entrycount);

	for (const auto& clb : mParent->spectralCallbacks())
		clb(entries, entrycount);
}

void LocalOutputSystem::commitShadingPoints(const OutputShadingPointEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitShadingPoints(entries, entrycount);
}

void LocalOutputSystem::commitFeedbacks(const OutputFeedbackEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitFeedbacks(entries, entrycount);
}

void LocalOutputSystem::commitCustomSpectrals(uint32 aov_id, StreamPipeline* pipeline, const OutputCustomSpectralEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitCustomSpectrals(aov_id, pipeline, entries, entrycount);
}

void LocalOutputSystem::commitCustom3D(uint32 aov_id, const OutputCustom3DEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitCustom3D(aov_id, entries, entrycount);
}

void LocalOutputSystem::commitCustom1D(uint32 aov_id, const OutputCustom1DEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitCustom1D(aov_id, entries, entrycount);
}

void LocalOutputSystem::commitCustomCounter(uint32 aov_id, const OutputCustomCounterEntry* entries, size_t entrycount)
{
	for (const auto& device : mLocalOutputDevices)
		device->commitCustomCounter(aov_id, entries, entrycount);
}

void LocalOutputSystem::addLocalOutputDevice(const std::shared_ptr<LocalOutputDevice>& device)
{
	PR_ASSERT(device, "Expected valid output device");
	mLocalOutputDevices.push_back(device);
}

std::shared_ptr<LocalOutputDevice> LocalOutputSystem::localOutputDevice(size_t i) const
{
	PR_ASSERT(i < mLocalOutputDevices.size(), "Invalid local output system configuration!");
	return mLocalOutputDevices[i];
}
} // namespace PR

#include "OutputSystem.h"
#include "LocalOutputSystem.h"
#include "OutputDevice.h"

namespace PR {
OutputSystem::OutputSystem(const Size2i& size)
	: mSize(size)
	, mLPE1DCount(0)
	, mLPE3DCount(0)
	, mLPECounterCount(0)
	, mLPESpectralCount(0)
{
}

OutputSystem::~OutputSystem() {}

void OutputSystem::addOutputDevice(const std::shared_ptr<OutputDevice>& device)
{
	PR_ASSERT(device, "Expected valid output device");
	mOutputDevices.push_back(device);
}

void OutputSystem::clear(bool force)
{
	for (const auto& device : mOutputDevices)
		device->clear(force);
}

std::shared_ptr<LocalOutputSystem> OutputSystem::createLocal(const RenderTile* tile, const Size2i& size) const
{
	auto local = std::make_shared<LocalOutputSystem>(tile, this, size);
	for (const auto& device : mOutputDevices)
		local->addLocalOutputDevice(device->createLocal(size));

	return local;
}

void OutputSystem::mergeLocal(const Point2i& p, const std::shared_ptr<LocalOutputSystem>& local, size_t iteration)
{
	local->beforeMerge();
	for (size_t i = 0; i < mOutputDevices.size(); ++i)
		mOutputDevices[i]->mergeLocal(p, local->localOutputDevice(i), iteration);
}

void OutputSystem::enableVarianceEstimation()
{
	enableSpectralChannel(AOV_OnlineVariance);
	enableSpectralChannel(AOV_OnlineMean);
}

void OutputSystem::enable1DChannel(AOV1D var)
{
	for (const auto& device : mOutputDevices)
		device->enable1DChannel(var);
}

void OutputSystem::enableCounterChannel(AOVCounter var)
{
	for (const auto& device : mOutputDevices)
		device->enableCounterChannel(var);
}

void OutputSystem::enable3DChannel(AOV3D var)
{
	for (const auto& device : mOutputDevices)
		device->enable3DChannel(var);
}

void OutputSystem::enableSpectralChannel(AOVSpectral var)
{
	for (const auto& device : mOutputDevices)
		device->enableSpectralChannel(var);
}

uint32 OutputSystem::registerLPE1DChannel(AOV1D var, const LightPathExpression& expr)
{
	const uint32 id = mLPE1DCount++;
	for (const auto& device : mOutputDevices)
		device->registerLPE1DChannel(var, expr, id);
	return id;
}

uint32 OutputSystem::registerLPECounterChannel(AOVCounter var, const LightPathExpression& expr)
{
	const uint32 id = mLPECounterCount++;
	for (const auto& device : mOutputDevices)
		device->registerLPECounterChannel(var, expr, id);
	return id;
}

uint32 OutputSystem::registerLPE3DChannel(AOV3D var, const LightPathExpression& expr)
{
	const uint32 id = mLPE3DCount++;
	for (const auto& device : mOutputDevices)
		device->registerLPE3DChannel(var, expr, id);
	return id;
}

uint32 OutputSystem::registerLPESpectralChannel(AOVSpectral var, const LightPathExpression& expr)
{
	const uint32 id = mLPESpectralCount++;
	for (const auto& device : mOutputDevices)
		device->registerLPESpectralChannel(var, expr, id);
	return id;
}

uint32 OutputSystem::registerCustom1DChannel(const std::string& str)
{
	if (mCustom1DChannelMap.count(str))
		return mCustom1DChannelMap.at(str);

	const uint32 id			 = mCustom1DChannelMap.size();
	mCustom1DChannelMap[str] = id;

	for (const auto& device : mOutputDevices)
		device->registerCustom1DChannel(str, id);

	return id;
}

uint32 OutputSystem::registerCustomCounterChannel(const std::string& str)
{
	if (mCustomCounterChannelMap.count(str))
		return mCustomCounterChannelMap.at(str);

	const uint32 id				  = mCustomCounterChannelMap.size();
	mCustomCounterChannelMap[str] = id;

	for (const auto& device : mOutputDevices)
		device->registerCustomCounterChannel(str, id);

	return id;
}

uint32 OutputSystem::registerCustom3DChannel(const std::string& str)
{
	if (mCustom3DChannelMap.count(str))
		return mCustom3DChannelMap.at(str);

	const uint32 id			 = mCustom3DChannelMap.size();
	mCustom3DChannelMap[str] = id;

	for (const auto& device : mOutputDevices)
		device->registerCustom3DChannel(str, id);

	return id;
}

uint32 OutputSystem::registerCustomSpectralChannel(const std::string& str)
{
	if (mCustomSpectralChannelMap.count(str))
		return mCustomSpectralChannelMap.at(str);

	const uint32 id				   = mCustomSpectralChannelMap.size();
	mCustomSpectralChannelMap[str] = id;

	for (const auto& device : mOutputDevices)
		device->registerCustomSpectralChannel(str, id);

	return id;
}

} // namespace PR

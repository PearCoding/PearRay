#include "IPDisplayDriver.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include "spectral/ToneMapper.h"

#include <OpenImageIO/imageio.h>

OIIO_NAMESPACE_USING;

namespace ip = boost::interprocess;
namespace PRU
{
	using namespace PR;

	IPDisplayDriver::IPDisplayDriver(const std::string& name) :
		mMapName(name), mSharedMemory(nullptr), mMappedRegion(nullptr), mRenderer(nullptr), mSaveData(nullptr)
	{
		PR_ASSERT(!name.empty());
	}

	IPDisplayDriver::~IPDisplayDriver()
	{
		deinit();
	}

	void IPDisplayDriver::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);
		deinit();

		try
		{
			const size_t shm_size =
				renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT*sizeof(float);

			mRenderer = renderer;

			mSharedMemory = new ip::shared_memory_object(ip::create_only, mMapName.c_str(), ip::read_write);
			mSharedMemory->truncate(shm_size);

			mMappedRegion = new ip::mapped_region(*mSharedMemory, ip::read_write);

			if(!mMappedRegion->get_address() || mMappedRegion->get_size() != shm_size)
			{
				deinit();
				PR_LOGGER.log(L_Fatal, M_System, "Got an invalid shared memory region");
				return;
			}
		}
		catch(ip::interprocess_exception &ex)
		{
			deinit();
			PR_LOGGER.logf(L_Fatal, M_System, "Couldn't initalize shared memory region: %s", ex.what());
			return;
		}

		mSaveData = new PR::uint8[renderer->width()*renderer->height()*3];

		clear();
	}

	void IPDisplayDriver::deinit()
	{
		if(mSharedMemory)
		{
			delete mSharedMemory;
			mSharedMemory = nullptr;	
		}

		if(mMappedRegion)
		{
			delete mMappedRegion;
			mMappedRegion = nullptr;	
		}

		ip::shared_memory_object::remove(mMapName.c_str());

		if(mSaveData)
		{
			delete[] mSaveData;
			mSaveData = nullptr;
		}
	}

	// TODO: No layer support
	void IPDisplayDriver::pushFragment(uint32 x, uint32 y, uint32 layer,
		uint32 sample, const Spectrum& s)
	{
		if (!mMappedRegion)
			return;
		
		float* data = (float*)mMappedRegion->get_address();

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT 
			+ x*Spectrum::SAMPLING_COUNT;

		const float t = 1.0f / (sample + 1.0f);
		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			data[index + i] = data[index + i] * (1-t) + s.value(i) * t;
		}
	}

	Spectrum IPDisplayDriver::fragment(uint32 x, uint32 y, uint32 layer) const
	{
		if (!mMappedRegion)
			return Spectrum();

		float* data = (float*)mMappedRegion->get_address();

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(x < mRenderer->width());

		const uint32 index = y*mRenderer->width()*Spectrum::SAMPLING_COUNT
			+ x*Spectrum::SAMPLING_COUNT;

		return Spectrum(&data[index]);
	}

	void IPDisplayDriver::clear()
	{
		if(mMappedRegion)
			std::memset(mMappedRegion->get_address(), 0,
			 mRenderer->width()*mRenderer->height()*Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	float* IPDisplayDriver::ptr() const
	{
		return mMappedRegion ? (float*)mMappedRegion->get_address() : nullptr;
	}

	bool IPDisplayDriver::save(const PR::ToneMapper& toneMapper, const std::string& file) const
	{
		PR_ASSERT(toneMapper.isByteMode());
		
		if(!mMappedRegion)
			return false;

		std::memset(mSaveData, 0, mRenderer->width() * mRenderer->height() * 3);
		toneMapper.exec((float*)mMappedRegion->get_address(), mSaveData);

		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;
		
		ImageSpec spec(mRenderer->width(), mRenderer->height(), 3, TypeDesc::UINT8);
		out->open(file, spec);
		out->write_image(TypeDesc::UINT8, mSaveData);
		out->close();
		ImageOutput::destroy(out);

		return true;
	}
}
#include "IPDisplayDriver.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include "spectral/RGBConverter.h"

#include <OpenImageIO/imageio.h>

OIIO_NAMESPACE_USING;

namespace ip = boost::interprocess;
namespace PRU
{
	using namespace PR;

	IPDisplayDriver::IPDisplayDriver(const std::string& name) :
		mMapName(name), mSharedMemory(nullptr), mMappedRegion(nullptr), mRenderer(nullptr)
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
			const size_t shm_size = renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT*sizeof(float);

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
	}

	// TODO: No layer support
	void IPDisplayDriver::pushFragment(uint32 x, uint32 y, uint32 layer, uint32 sample, const Spectrum& s)
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
			std::memset(mMappedRegion->get_address(), 0, mRenderer->width()*mRenderer->height()*Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	float* IPDisplayDriver::ptr() const
	{
		return mMappedRegion ? (float*)mMappedRegion->get_address() : nullptr;
	}

	bool IPDisplayDriver::save(const std::string& file) const
	{
		unsigned char* rgb = new unsigned char[mRenderer->width() * mRenderer->height() * 3];
		std::memset(rgb, 0, mRenderer->width() * mRenderer->height() * 3);

		for(uint32 y = mRenderer->cropPixelOffsetY();
			y < mRenderer->cropPixelOffsetY() + mRenderer->renderHeight();
			++y)
		{
			for(uint32 x = mRenderer->cropPixelOffsetX();
				x < mRenderer->cropPixelOffsetX() + mRenderer->renderWidth();
				++x)
			{
				float r, g, b;
				PR::RGBConverter::convert(fragment(x,y,0), r,g,b);
				//PR::RGBConverter::gamma(r,g,b);

				rgb[y*mRenderer->width()*3 + x*3] = static_cast<PR::uint8>(r*255);
				rgb[y*mRenderer->width()*3 + x*3 + 1] = static_cast<PR::uint8>(g*255);
				rgb[y*mRenderer->width()*3 + x*3 + 2] = static_cast<PR::uint8>(b*255);
			}
		}

		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;
		
		ImageSpec spec(mRenderer->width(), mRenderer->height(), 3, TypeDesc::UINT8);
		out->open(file, spec);
		out->write_image(TypeDesc::UINT8, rgb);
		out->close();
		ImageOutput::destroy(out);

		delete[] rgb;

		return true;
	}
}
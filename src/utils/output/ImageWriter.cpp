#include "ImageWriter.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include <OpenImageIO/imageio.h>


OIIO_NAMESPACE_USING;

namespace PRU
{
	using namespace PR;

	ImageWriter::ImageWriter() :
		mData(nullptr), mSaveData(nullptr), mRenderer(nullptr)
	{
	}

	ImageWriter::~ImageWriter()
	{
		deinit();
	}

	void ImageWriter::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);

		// Delete if resolution changed.
		if(mRenderer && mData &&
			(mRenderer->width() != renderer->width() || mRenderer->height() != renderer->height()))
		{
			delete[] mData;
			mData = nullptr;

			if(mSaveData)
			{
				delete[] mSaveData;
				mSaveData = nullptr;
			}
		}

		mRenderer = renderer;

		if(!mData)
		{
			mData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];
			mSaveData = new PR::uint8[renderer->width()*renderer->height()*3];
		}
	}

	void ImageWriter::deinit()
	{
		if (mData)
		{
			delete[] mData;
			mData = nullptr;
		}

		if(mSaveData)
		{
			delete[] mSaveData;
			mSaveData = nullptr;
		}

		if(!mRenderer)
			return;
	}

	// TODO: Implement different channel modes
	bool ImageWriter::save(PR::ToneMapper& toneMapper, const std::string& file,
			IM_ChannelSettingSpec* specSett,
			const std::vector<IM_ChannelSetting1D>& ch1d,
			const std::vector<IM_ChannelSetting3D>& ch3d) const
	{
		PR_ASSERT(!toneMapper.isByteMode());

		const uint32 channelCount = (specSett ? 3 : 0) + ch1d.size() + ch3d.size()*3;
		if(channelCount == 0)
			return false;

		ImageSpec spec(mRenderer->width(), mRenderer->height(), channelCount, TypeDesc::FLOAT);
		// Channel names
		spec.channelnames.clear ();
		if(specSett)
		{
			spec.channelnames.push_back("R");
			spec.channelnames.push_back("G");
			spec.channelnames.push_back("B");
		}
		for(const IM_ChannelSetting3D& sett : ch3d)
		{
			spec.channelnames.push_back(sett.Name[0]);
			spec.channelnames.push_back(sett.Name[1]);
			spec.channelnames.push_back(sett.Name[2]);
		}
		for(const IM_ChannelSetting1D& sett : ch1d)
		{
			spec.channelnames.push_back(sett.Name);
		}
		spec.attribute ("Software", "PearRay " PR_VERSION_STRING);

		// Spectral
		if(specSett)
		{
			std::memset(mSaveData, 0, mRenderer->width() * mRenderer->height() * 3);
			toneMapper.setColorMode(specSett->TCM);
			toneMapper.setGammaMode(specSett->TGM);
			toneMapper.setMapperMode(specSett->TMM);
			toneMapper.exec(mData, mSaveData);// RGB
		}
		
		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;
		
		float* line = new float[channelCount * mRenderer->width()];

		out->open(file, spec);
		for (uint32 y = 0; y < mRenderer->height(); ++y)
		{
			for(uint32 x = 0; x < mRenderer->width(); ++x)
			{
				uint32 id = x*channelCount;
				const uint32 id1d = y * mRenderer->width() + x;
				const uint32 id3d = id1d*3;

				if(specSett)
				{
					line[id] = mSaveData[id3d];
					line[id+1] = mSaveData[id3d+1];
					line[id+2] = mSaveData[id3d+2];
					id += 3;
				}

				for(const IM_ChannelSetting3D& sett : ch3d)
				{
					PM::avec3 a = sett.Channel->ptr()[id1d];
					line[id] = a[0];
					line[id+1] = a[1];
					line[id+2] = a[2];
					id += 3;
				}

				for(const IM_ChannelSetting1D& sett : ch1d)
				{
					line[id] = sett.Channel->ptr()[id1d];
					id += 1;
				}
			}
			out->write_scanline(y, 0, TypeDesc::FLOAT, line);
		}
		out->close();

		delete[] line;
		ImageOutput::destroy(out);

		return true;
	}

	bool ImageWriter::save_spectral(const std::string& file,
			PR::OutputSpectral* spec) const
	{
		return false;//TODO
	}
}
#include "ImageWriter.h"
#include "renderer/Renderer.h"
#include "Logger.h"

#include <OpenImageIO/imageio.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>


OIIO_NAMESPACE_USING;

namespace PRU
{
	using namespace PR;

	ImageWriter::ImageWriter() :
		mSpectralData(nullptr), mRGBData(nullptr), mRenderer(nullptr)
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
		if(mRenderer && mSpectralData &&
			(mRenderer->width() != renderer->width() || mRenderer->height() != renderer->height()))
		{
			delete[] mSpectralData;
			mSpectralData = nullptr;

			if(mRGBData)
			{
				delete[] mRGBData;
				mRGBData = nullptr;
			}
		}

		mRenderer = renderer;

		if(!mSpectralData)
		{
			mSpectralData = new float[renderer->width()*renderer->height()*Spectrum::SAMPLING_COUNT];
			mRGBData = new float[renderer->width()*renderer->height()*3];
		}
	}

	void ImageWriter::deinit()
	{
		if (mSpectralData)
		{
			delete[] mSpectralData;
			mSpectralData = nullptr;
		}

		if(mRGBData)
		{
			delete[] mRGBData;
			mRGBData = nullptr;
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
			
			if(specSett->TGM == TGM_SRGB)
				spec.attribute("oiio:ColorSpace", "srgb");
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
		if(specSett && specSett->Channel)
		{
			for(uint32 i = 0; i < mRenderer->width() * mRenderer->height(); ++i)
			{
				specSett->Channel->ptr()[i].copyTo(&mSpectralData[i*Spectrum::SAMPLING_COUNT]);
			}
			
			toneMapper.setColorMode(specSett->TCM);
			toneMapper.setGammaMode(specSett->TGM);
			toneMapper.setMapperMode(specSett->TMM);
			toneMapper.exec(mSpectralData, mRGBData);// RGB
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
					line[id] = mRGBData[id3d];
					line[id+1] = mRGBData[id3d+1];
					line[id+2] = mRGBData[id3d+2];
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
		if(!spec)
			return false;
		
		namespace io = boost::iostreams;
		io::filtering_ostream out;
    	out.push(io::zlib_compressor());
    	out.push(io::file_sink(file));

		out.put('P').put('R').put('S').put('4').put('2');

		uint32 tmp = Spectrum::SAMPLING_COUNT;
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mRenderer->width();
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mRenderer->height();
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));

		float buf[Spectrum::SAMPLING_COUNT];
		for (uint32 i = 0; i < mRenderer->height() * mRenderer->width(); ++i)
		{
			const Spectrum& s = spec->ptr()[i];
			s.copyTo(buf);
			out.write(reinterpret_cast<const char*>(buf),
				Spectrum::SAMPLING_COUNT * sizeof(float));
		}

		return true;
	}
}
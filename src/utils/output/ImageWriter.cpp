#include "ImageWriter.h"
#include "renderer/RenderContext.h"
#include "Logger.h"

#include <OpenImageIO/imageio.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

OIIO_NAMESPACE_USING;

namespace PR
{
	ImageWriter::ImageWriter() :
		mRGBData(nullptr), mRenderer(nullptr)
	{
	}

	ImageWriter::~ImageWriter()
	{
		deinit();
	}

	void ImageWriter::init(RenderContext* renderer)
	{
		// Delete if resolution changed.
		if(mRenderer && (!renderer ||
			(mRenderer->width() != renderer->width() ||
			 mRenderer->height() != renderer->height())))
		{
			if(mRGBData)
			{
				delete[] mRGBData;
				mRGBData = nullptr;
			}
		}

		mRenderer = renderer;

		if(mRenderer && !mRGBData)
			mRGBData = new float[mRenderer->width()*mRenderer->height()*3];
	}

	void ImageWriter::deinit()
	{
		if(mRGBData)
		{
			delete[] mRGBData;
			mRGBData = nullptr;
		}

		mRenderer = nullptr;
	}

	bool ImageWriter::save(PR::ToneMapper& toneMapper, const std::string& file,
			IM_ChannelSettingSpec* specSett,
			const std::vector<IM_ChannelSetting1D>& ch1d,
			const std::vector<IM_ChannelSettingCounter>& chcounter,
			const std::vector<IM_ChannelSetting3D>& ch3d) const
	{
		if(!mRenderer)
			return false;
	
		const uint32 rw = mRenderer->width();
		const uint32 rh = mRenderer->height();
		const uint32 cx = mRenderer->offsetX();
		const uint32 cy = mRenderer->offsetY();

		const uint32 channelCount = (specSett ? 3 : 0) + ch1d.size() + ch3d.size()*3;
		if(channelCount == 0)
			return false;

		ImageSpec spec(rw, rh,
			channelCount, TypeDesc::FLOAT);
		spec.full_x = 0;
		spec.full_y = 0;
		spec.full_width = mRenderer->fullWidth();
		spec.full_height = mRenderer->fullHeight();
		spec.x = cx;
		spec.y = cy;

		// Channel names
		spec.channelnames.clear ();
		if(specSett)
		{
			spec.channelnames.push_back("R");
			spec.channelnames.push_back("G");
			spec.channelnames.push_back("B");
			
			if(specSett->TGM == TGM_SRGB)
				spec.attribute("oiio:ColorSpace", "sRGB");
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
		for(const IM_ChannelSettingCounter& sett : chcounter)
		{
			spec.channelnames.push_back(sett.Name);
		}
		
		spec.attribute ("Software", "PearRay " PR_VERSION_STRING);
		spec.attribute ("IPTC:ProgramVersion", PR_VERSION_STRING);

		// Create file
		ImageOutput* out = ImageOutput::create(file);
		if(!out)
			return false;

		// Spectral
		if(specSett && mRenderer->output()->getSpectralChannel())
		{			
			toneMapper.setColorMode(specSett->TCM);
			toneMapper.setGammaMode(specSett->TGM);
			toneMapper.setMapperMode(specSett->TMM);
			toneMapper.exec(mRenderer->output()->getSpectralChannel()->ptr(), mRGBData);// RGB
		}
		
		// Calculate maximums for some mapper techniques
		float invMax3d[OutputMap::V_3D_COUNT];
		std::fill_n(invMax3d, OutputMap::V_3D_COUNT, 0);
		float invMax1d[OutputMap::V_1D_COUNT];
		std::fill_n(invMax1d, OutputMap::V_1D_COUNT, 0);
		float invMaxCounter[OutputMap::V_COUNTER_COUNT];
		std::fill_n(invMax1d, OutputMap::V_COUNTER_COUNT, 0);

		for(const IM_ChannelSetting3D& sett : ch3d)
		{
			const auto& channel = mRenderer->output()->getChannel(sett.Variable);
			if(sett.TMM != TMM_Normalized || !channel)
				continue;

			for (uint32 y = 0; y < rh; ++y)
			{
				for(uint32 x = 0; x < rw; ++x)
				{
					invMax3d[sett.Variable] =
						PM::pm_Max(invMax3d[sett.Variable],
							PM::pm_MagnitudeSqr(channel->getFragmentBounded(x,y)));
				}
			}

			if(invMax3d[sett.Variable] > PM_EPSILON)
				invMax3d[sett.Variable] = 1.0f / std::sqrt(invMax3d[sett.Variable]);
		}

		for(const IM_ChannelSetting1D& sett : ch1d)
		{
			const auto& channel = mRenderer->output()->getChannel(sett.Variable);
			if(sett.TMM != TMM_Normalized || !channel)
				continue;

			for (uint32 y = 0; y < rh; ++y)
			{
				for(uint32 x = 0; x < rw; ++x)
				{
					invMax1d[sett.Variable] =
						PM::pm_Max(invMax1d[sett.Variable], channel->getFragmentBounded(x,y));
				}
			}

			if(invMax1d[sett.Variable] > PM_EPSILON)
				invMax1d[sett.Variable] = 1.0f / invMax1d[sett.Variable];
		}

		for(const IM_ChannelSettingCounter& sett : chcounter)
		{
			const auto& channel = mRenderer->output()->getChannel(sett.Variable);
			if(sett.TMM != TMM_Normalized || !channel)
				continue;

			for (uint32 y = 0; y < rh; ++y)
			{
				for(uint32 x = 0; x < rw; ++x)
				{
					invMaxCounter[sett.Variable] =
						PM::pm_Max<uint64>(invMaxCounter[sett.Variable], channel->getFragmentBounded(x,y));
				}
			}

			if(invMaxCounter[sett.Variable] > PM_EPSILON)
				invMaxCounter[sett.Variable] = 1.0f / invMaxCounter[sett.Variable];
		}

		// Write content		
		float* line = new float[channelCount * rw];
		if(!line)// TODO: Add single token variant!
		{
			PR_LOGGER.log(L_Error, M_System, "Not enough memory for image output!");
			ImageOutput::destroy(out);
			return false;
		}

		out->open(file, spec);
		for (uint32 y = 0; y < rh; ++y)
		{
			for(uint32 x = 0; x < rw; ++x)
			{
				uint32 id = x*channelCount;
				const uint32 id1d = y * rw + x;
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
					const auto& channel = mRenderer->output()->getChannel(sett.Variable);
					if(channel)
					{
						const PM::vec3& a = channel->ptr()[id1d];

						float r = PM::pm_GetX(a);
						float g = PM::pm_GetY(a);
						float b = PM::pm_GetZ(a);
						switch(sett.TMM)
						{
						case TMM_None:
						case TMM_Simple_Reinhard:
							break;
						case TMM_Normalized:
							r *= invMax3d[sett.Variable]; g *= invMax3d[sett.Variable]; b *= invMax3d[sett.Variable];
							break;
						case TMM_Clamp:
							r = PM::pm_Max(std::abs(r), 0.0f);
							g = PM::pm_Max(std::abs(g), 0.0f);
							b = PM::pm_Max(std::abs(b), 0.0f);
							break;
						case TMM_Abs:
							r = std::abs(r);
							g = std::abs(g);
							b = std::abs(b);
							break;
						case TMM_Positive:
							r = PM::pm_Max(r, 0.0f);
							g = PM::pm_Max(g, 0.0f);
							b = PM::pm_Max(b, 0.0f);
							break;
						case TMM_Negative:
							r = PM::pm_Max(-r, 0.0f);
							g = PM::pm_Max(-g, 0.0f);
							b = PM::pm_Max(-b, 0.0f);
							break;
						case TMM_Spherical:
							r = 0.5f + 0.5f * std::atan2(b, r) * PM_INV_PI_F;
							g = 0.5f - std::asin(-g) * PM_INV_PI_F;
							b = 0;
							break;
						}

						line[id] = r;
						line[id+1] = g;
						line[id+2] = b;
					}

					id += 3;
				}

				for(const IM_ChannelSetting1D& sett : ch1d)
				{
					const auto& channel = mRenderer->output()->getChannel(sett.Variable);
					if(channel)
					{
						float r = channel->ptr()[id1d];
						switch(sett.TMM)
						{
						case TMM_None:
						case TMM_Simple_Reinhard:
						case TMM_Spherical:
							break;
						case TMM_Normalized:
							r *= invMax1d[sett.Variable];
							break;
						case TMM_Clamp:
							r = PM::pm_Max(std::abs(r), 0.0f);
							break;
						case TMM_Abs:
							r = std::abs(r);
							break;
						case TMM_Positive:
							r = PM::pm_Max(r, 0.0f);
							break;
						case TMM_Negative:
							r = PM::pm_Max(-r, 0.0f);
							break;
						}

						line[id] = r;
					}

					id += 1;
				}

				for(const IM_ChannelSettingCounter& sett : chcounter)
				{
					const auto& channel = mRenderer->output()->getChannel(sett.Variable);
					if(channel)
					{
						float r = channel->ptr()[id1d];
						switch(sett.TMM)
						{
						case TMM_Normalized:
							r *= invMaxCounter[sett.Variable];
							break;
						}

						line[id] = r;
					}

					id += 1;
				}
			}
			out->write_scanline(y+cy, 0, TypeDesc::FLOAT, line);
		}
		out->close();

		delete[] line;
		ImageOutput::destroy(out);

		return true;
	}

	bool ImageWriter::save_spectral(const std::string& file,
			PR::OutputSpectral* spec) const
	{
		if(!spec || !mRenderer)
			return false;
		
		namespace io = boost::iostreams;
		io::filtering_ostream out;
    	out.push(io::zlib_compressor());
    	out.push(io::file_sink(file));

		out.put('P').put('R').put('4').put('2');

		uint32 tmp = Spectrum::SAMPLING_COUNT;
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mRenderer->width();
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mRenderer->height();
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));

		for (uint32 i = 0; i < mRenderer->height() * mRenderer->width(); ++i)
			out.write(reinterpret_cast<const char*>(&spec->ptr()[i]),
				Spectrum::SAMPLING_COUNT * sizeof(float));

		return true;
	}
}
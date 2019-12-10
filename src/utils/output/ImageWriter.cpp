#include "ImageWriter.h"
#include "Logger.h"
#include "SpectralFile.h"
#include "Version.h"
#include "buffer/OutputBuffer.h"
#include "renderer/RenderContext.h"

#include <OpenImageIO/imageio.h>
#include <boost/filesystem.hpp>

OIIO_NAMESPACE_USING

namespace PR {
ImageWriter::ImageWriter()
	: mRGBData(nullptr)
	, mRenderer(nullptr)
{
}

ImageWriter::~ImageWriter()
{
	deinit();
}

void ImageWriter::init(const std::shared_ptr<RenderContext>& renderer)
{
	// Delete if resolution changed.
	if (mRenderer
		&& (!renderer
			|| mRenderer->width() != renderer->width()
			|| mRenderer->height() != renderer->height())) {
		if (mRGBData) {
			delete[] mRGBData;
			mRGBData = nullptr;
		}
	}

	mRenderer = renderer;

	if (mRenderer && !mRGBData)
		mRGBData = new float[mRenderer->width() * mRenderer->height() * 3];
}

void ImageWriter::deinit()
{
	if (mRGBData) {
		delete[] mRGBData;
		mRGBData = nullptr;
	}

	mRenderer = nullptr;
}

bool ImageWriter::save(ToneMapper& toneMapper, const std::wstring& file,
					   const std::vector<IM_ChannelSettingSpec>& chSpec,
					   const std::vector<IM_ChannelSetting1D>& ch1d,
					   const std::vector<IM_ChannelSettingCounter>& chcounter,
					   const std::vector<IM_ChannelSetting3D>& ch3d) const
{
	if (!mRenderer)
		return false;

	const uint32 rw = mRenderer->width();
	const uint32 rh = mRenderer->height();
	const uint32 cx = mRenderer->offsetX();
	const uint32 cy = mRenderer->offsetY();

	const size_t channelCount = chSpec.size() * 3 + ch1d.size() + chcounter.size() + ch3d.size() * 3;
	if (channelCount == 0)
		return false;

	ImageSpec spec(rw, rh,
				   (int)channelCount, TypeDesc::FLOAT);
	spec.full_x		 = 0;
	spec.full_y		 = 0;
	spec.full_width  = mRenderer->settings().filmWidth;
	spec.full_height = mRenderer->settings().filmHeight;
	spec.x			 = cx;
	spec.y			 = cy;

	// Channel names
	spec.channelnames.clear();
	for (const IM_ChannelSettingSpec& sett : chSpec) {
		if (sett.Name.empty()) {
			spec.channelnames.push_back("R");
			spec.channelnames.push_back("G");
			spec.channelnames.push_back("B");
		} else {
			spec.channelnames.push_back(sett.Name + ".R");
			spec.channelnames.push_back(sett.Name + ".G");
			spec.channelnames.push_back(sett.Name + ".B");
		}

		if (sett.TGM == TGM_SRGB)
			spec.attribute("oiio:ColorSpace", "sRGB");
	}

	for (const IM_ChannelSetting3D& sett : ch3d) {
		spec.channelnames.push_back(sett.Name[0]);
		spec.channelnames.push_back(sett.Name[1]);
		spec.channelnames.push_back(sett.Name[2]);
	}

	for (const IM_ChannelSetting1D& sett : ch1d) {
		spec.channelnames.push_back(sett.Name);
	}

	for (const IM_ChannelSettingCounter& sett : chcounter) {
		spec.channelnames.push_back(sett.Name);
	}

	spec.attribute("Software", "PearRay " PR_VERSION_STRING);
	spec.attribute("IPTC:ProgramVersion", PR_VERSION_STRING);

	const std::string utfFilename = boost::filesystem::path(file).generic_string();
// Create file
#if OIIO_PLUGIN_VERSION >= 22
	std::unique_ptr<ImageOutput> out = ImageOutput::create(utfFilename);
#else
	ImageOutput* out = ImageOutput::create(utfFilename);
#endif
	if (!out)
		return false;

	// Calculate maximums for some mapper techniques
	float invMax3d[AOV_3D_COUNT];
	std::fill_n(invMax3d, AOV_3D_COUNT, 0.0f);
	float invMax1d[AOV_1D_COUNT];
	std::fill_n(invMax1d, AOV_1D_COUNT, 0.0f);
	float invMaxCounter[AOV_COUNTER_COUNT];
	std::fill_n(invMax1d, AOV_COUNTER_COUNT, 0.0f);

	const OutputBufferData& data = mRenderer->output()->data();

	for (const IM_ChannelSetting3D& sett : ch3d) {
		std::shared_ptr<FrameBufferFloat> channel;
		if (sett.LPE < 0)
			channel = data.getInternalChannel_3D(sett.Variable);
		else
			channel = data.getLPEChannel_3D(sett.Variable, sett.LPE);

		if (sett.TMM != TMM_Normalized || !channel)
			continue;

		for (uint32 y = 0; y < rh; ++y) {
			for (uint32 x = 0; x < rw; ++x) {
				Vector3f v(channel->getFragment(x, y, 0),
						   channel->getFragment(x, y, 1),
						   channel->getFragment(x, y, 2));

				invMax3d[sett.Variable] = std::max(invMax3d[sett.Variable],
												   v.squaredNorm());
			}
		}

		if (invMax3d[sett.Variable] > PR_EPSILON)
			invMax3d[sett.Variable] = 1.0f / std::sqrt(invMax3d[sett.Variable]);
	}

	for (const IM_ChannelSetting1D& sett : ch1d) {
		std::shared_ptr<FrameBufferFloat> channel;
		if (sett.LPE < 0)
			channel = data.getInternalChannel_1D(sett.Variable);
		else
			channel = data.getLPEChannel_1D(sett.Variable, sett.LPE);

		if (sett.TMM != TMM_Normalized || !channel)
			continue;

		for (uint32 y = 0; y < rh; ++y) {
			for (uint32 x = 0; x < rw; ++x) {
				invMax1d[sett.Variable] = std::max(invMax1d[sett.Variable],
												   channel->getFragment(x, y, 0));
			}
		}

		if (invMax1d[sett.Variable] > PR_EPSILON)
			invMax1d[sett.Variable] = 1.0f / invMax1d[sett.Variable];
	}

	for (const IM_ChannelSettingCounter& sett : chcounter) {
		std::shared_ptr<FrameBufferUInt32> channel;
		if (sett.LPE < 0)
			channel = data.getInternalChannel_Counter(sett.Variable);
		else
			channel = data.getLPEChannel_Counter(sett.Variable, sett.LPE);

		if (sett.TMM != TMM_Normalized || !channel)
			continue;

		for (uint32 y = 0; y < rh; ++y) {
			for (uint32 x = 0; x < rw; ++x) {
				invMaxCounter[sett.Variable] = std::max<float>(invMaxCounter[sett.Variable],
															   static_cast<float>(channel->getFragment(x, y, 0)));
			}
		}

		if (invMaxCounter[sett.Variable] > PR_EPSILON)
			invMaxCounter[sett.Variable] = 1.0f / invMaxCounter[sett.Variable];
	}

	// Write content
	float* line = new float[channelCount * rw];
	if (!line) { // TODO: Add single token variant!
		PR_LOG(L_ERROR) << "Not enough memory for image output!" << std::endl;

#if OIIO_PLUGIN_VERSION < 22
		ImageOutput::destroy(out);
#endif
		return false;
	}

	out->open(utfFilename, spec);
	for (uint32 y = 0; y < rh; ++y) {
		for (uint32 x = 0; x < rw; ++x) {
			size_t id = x * channelCount;

			// Spectral
			for (const IM_ChannelSettingSpec& sett : chSpec) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_Spectral();
				else
					channel = data.getLPEChannel_Spectral(sett.LPE);

				const float* ptr = channel->ptr();
				toneMapper.setColorMode(sett.TCM);
				toneMapper.setGammaMode(sett.TGM);
				toneMapper.setMapperMode(sett.TMM);
				toneMapper.map(&ptr[y * channel->heightPitch() + x * channel->widthPitch()],
							   channel->channels(),
							   &line[id], 3, 1); // RGB

				id += 3;
			}

			for (const IM_ChannelSetting3D& sett : ch3d) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_3D(sett.Variable);
				else
					channel = data.getLPEChannel_3D(sett.Variable, sett.LPE);

				if (channel) {
					Vector3f a(channel->getFragment(x, y, 0),
							   channel->getFragment(x, y, 1),
							   channel->getFragment(x, y, 2));

					float r = a(0);
					float g = a(1);
					float b = a(2);
					switch (sett.TMM) {
					default:
					case TMM_None:
					case TMM_Simple_Reinhard:
						break;
					case TMM_Normalized:
						r *= invMax3d[sett.Variable];
						g *= invMax3d[sett.Variable];
						b *= invMax3d[sett.Variable];
						break;
					case TMM_Clamp:
						r = std::max(std::abs(r), 0.0f);
						g = std::max(std::abs(g), 0.0f);
						b = std::max(std::abs(b), 0.0f);
						break;
					case TMM_Abs:
						r = std::abs(r);
						g = std::abs(g);
						b = std::abs(b);
						break;
					case TMM_Positive:
						r = std::max(r, 0.0f);
						g = std::max(g, 0.0f);
						b = std::max(b, 0.0f);
						break;
					case TMM_Negative:
						r = std::max(-r, 0.0f);
						g = std::max(-g, 0.0f);
						b = std::max(-b, 0.0f);
						break;
					case TMM_Spherical:
						r = 0.5f + 0.5f * std::atan2(b, r) * PR_1_PI;
						g = 0.5f - std::asin(-g) * PR_1_PI;
						b = 0;
						break;
					}

					line[id]	 = r;
					line[id + 1] = g;
					line[id + 2] = b;
				}

				id += 3;
			}

			for (const IM_ChannelSetting1D& sett : ch1d) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_1D(sett.Variable);
				else
					channel = data.getLPEChannel_1D(sett.Variable, sett.LPE);

				if (channel) {
					float r = channel->getFragment(x, y, 0);
					switch (sett.TMM) {
					default:
					case TMM_None:
					case TMM_Simple_Reinhard:
					case TMM_Spherical:
						break;
					case TMM_Normalized:
						r *= invMax1d[sett.Variable];
						break;
					case TMM_Clamp:
						r = std::max(std::abs(r), 0.0f);
						break;
					case TMM_Abs:
						r = std::abs(r);
						break;
					case TMM_Positive:
						r = std::max(r, 0.0f);
						break;
					case TMM_Negative:
						r = std::max(-r, 0.0f);
						break;
					}

					line[id] = r;
				}

				id += 1;
			}

			for (const IM_ChannelSettingCounter& sett : chcounter) {
				std::shared_ptr<FrameBufferUInt32> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_Counter(sett.Variable);
				else
					channel = data.getLPEChannel_Counter(sett.Variable, sett.LPE);

				if (channel) {
					float r = static_cast<float>(channel->getFragment(x, y, 0));
					switch (sett.TMM) {
					default:
						break;
					case TMM_Normalized:
						r *= invMaxCounter[sett.Variable];
						break;
					}

					line[id] = r;
				}

				id += 1;
			}
		}
		out->write_scanline(y + cy, 0, TypeDesc::FLOAT, line);
	}
	out->close();

	delete[] line;

#if OIIO_PLUGIN_VERSION < 22
	ImageOutput::destroy(out);
#endif

	return true;
}

bool ImageWriter::save_spectral(const std::wstring& file,
								const std::shared_ptr<FrameBufferFloat>& spec,
								bool compress) const
{
	if (!spec || !mRenderer)
		return false;

	SpectralFile specFile(mRenderer->spectrumDescriptor(),
						  mRenderer->width(), mRenderer->height(), spec->ptr(), false);
	specFile.save(file, compress);

	return true;
}
} // namespace PR

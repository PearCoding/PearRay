#include "ImageWriter.h"
#include "Logger.h"
#include "buffer/FrameBufferSystem.h"
#include "config/Build.h"
#include "renderer/RenderContext.h"
#include "spectral/SpectralFile.h"

#include <OpenImageIO/imageio.h>
#include <boost/filesystem.hpp>

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
			|| mRenderer->viewSize() != renderer->viewSize())) {
		if (mRGBData) {
			delete[] mRGBData;
			mRGBData = nullptr;
		}
	}

	mRenderer = renderer;

	if (mRenderer && !mRGBData)
		mRGBData = new float[mRenderer->viewSize().area() * 3];
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
					   const std::vector<IM_ChannelSetting3D>& ch3d,
					   const IM_SaveOptions& options) const
{
	if (!mRenderer)
		return false;

	const Size2i viewSize = mRenderer->viewSize();
	const Point2i viewOff = mRenderer->viewOffset();

	const size_t channelCount = chSpec.size() * 3 + ch1d.size() + chcounter.size() + ch3d.size() * 3;
	if (channelCount == 0)
		return false;

	OIIO::ImageSpec spec(viewSize.Width, viewSize.Height,
						 (int)channelCount, OIIO::TypeDesc::FLOAT);
	spec.full_x		 = 0;
	spec.full_y		 = 0;
	spec.full_width	 = mRenderer->settings().filmWidth;
	spec.full_height = mRenderer->settings().filmHeight;
	spec.x			 = viewOff.x();
	spec.y			 = viewOff.y();

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

	const std::string versionStr = Build::getVersionString();
	spec.attribute("Software", "PearRay " + versionStr);
	spec.attribute("IPTC:ProgramVersion", versionStr);
	if (options.WriteMeta) {
		spec.attribute("PearRay:IterationCount", options.IterationMeta);
		spec.attribute("PearRay:TimeSpent", (uint32)options.TimeMeta);
	}

	const std::string utfFilename = boost::filesystem::path(file).generic_string();
	// Create file
	auto out = OIIO::ImageOutput::create(utfFilename);
	if (!out)
		return false;

	const FrameBufferContainer& data = mRenderer->output()->data();

	// Write content
	float* line = new float[channelCount * viewSize.Width];
	if (!line) { // TODO: Add single token variant!
		PR_LOG(L_ERROR) << "Not enough memory for image output!" << std::endl;

#if OIIO_PLUGIN_VERSION < 22
		OIIO::ImageOutput::destroy(out);
#endif
		return false;
	}

	out->open(utfFilename, spec);
	for (Size1i y = 0; y < viewSize.Height; ++y) {
		PR_OPT_LOOP
		for (Size1i x = 0; x < viewSize.Width; ++x) {
			size_t id		= x * channelCount;
			const Point2i p = Point2i(x, y);

			// Spectral
			for (const IM_ChannelSettingSpec& sett : chSpec) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_Spectral();
				else
					channel = data.getLPEChannel_Spectral(sett.LPE);

				const float* ptr = channel->ptr();
				toneMapper.setColorMode(sett.TCM);
				toneMapper.map(&ptr[y * channel->heightPitch() + x * channel->widthPitch()],
							   &line[id], 3, 1); // RGB

				line[id] *= options.SpectralFactor;
				line[id + 1] *= options.SpectralFactor;
				line[id + 2] *= options.SpectralFactor;

				id += 3;
			}

			for (const IM_ChannelSetting3D& sett : ch3d) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_3D(sett.Variable);
				else
					channel = data.getLPEChannel_3D(sett.Variable, sett.LPE);

				if (channel) {
					line[id]	 = channel->getFragment(p, 0);
					line[id + 1] = channel->getFragment(p, 1);
					line[id + 2] = channel->getFragment(p, 2);
				}

				id += 3;
			}

			for (const IM_ChannelSetting1D& sett : ch1d) {
				std::shared_ptr<FrameBufferFloat> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_1D(sett.Variable);
				else
					channel = data.getLPEChannel_1D(sett.Variable, sett.LPE);

				if (channel)
					line[id] = channel->getFragment(p, 0);

				id += 1;
			}

			for (const IM_ChannelSettingCounter& sett : chcounter) {
				std::shared_ptr<FrameBufferUInt32> channel;
				if (sett.LPE < 0)
					channel = data.getInternalChannel_Counter(sett.Variable);
				else
					channel = data.getLPEChannel_Counter(sett.Variable, sett.LPE);

				if (channel)
					line[id] = static_cast<float>(channel->getFragment(p, 0));

				id += 1;
			}
		}
		out->write_scanline(y + viewOff.y(), 0, OIIO::TypeDesc::FLOAT, line);
	}
	out->close();

	delete[] line;

#if OIIO_PLUGIN_VERSION < 22
	OIIO::ImageOutput::destroy(out);
#endif

	return true;
}
} // namespace PR

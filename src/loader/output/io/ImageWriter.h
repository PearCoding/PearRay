#pragma once

#include "buffer/FrameBuffer.h"
#include "output/AOV.h"
#include "spectral/ToneMapper.h"

#include <filesystem>
#include <vector>

namespace PR {
struct IM_ChannelSetting1D {
	std::string Name;
	AOV1D Variable;
	std::string LPE_S;
	int LPE = -1;
	std::string Custom;
	int CustomID = -1;
};

struct IM_ChannelSettingCounter {
	std::string Name;
	AOVCounter Variable;
	std::string LPE_S;
	int LPE = -1;
	std::string Custom;
	int CustomID = -1;
};

struct IM_ChannelSetting3D {
	std::string Name[3];
	AOV3D Variable;
	std::string LPE_S;
	int LPE = -1;
	std::string Custom;
	int CustomID = -1;
};

struct IM_ChannelSettingSpec {
	std::string Name;
	AOVSpectral Variable = AOV_Output;
	ToneColorMode TCM;
	std::string LPE_S;
	int LPE = -1;
	std::string Custom;
	int CustomID = -1;
	bool IsRaw = false;
};

struct IM_SaveOptions {
	uint32 IterationMeta = 0;
	uint64 TimeMeta		 = 0; // Seconds
	bool WriteMeta		 = false;
	float SpectralFactor = 1.0f; // Optional weight factor
};

class FrameOutputDevice;
class RenderContext;

/// Image writer to be used with the FrameOutputDevice
class PR_LIB_LOADER ImageWriter {
	PR_CLASS_NON_COPYABLE(ImageWriter);

public:
	ImageWriter();
	virtual ~ImageWriter();

	void init(const std::shared_ptr<RenderContext>& renderer);
	void deinit();

	bool save(FrameOutputDevice* outputDevice,
			  ToneMapper& toneMapper, const std::filesystem::path& file,
			  const std::vector<IM_ChannelSettingSpec>& spec,
			  const std::vector<IM_ChannelSetting1D>& ch1d,
			  const std::vector<IM_ChannelSettingCounter>& chcounter,
			  const std::vector<IM_ChannelSetting3D>& ch3d,
			  const IM_SaveOptions& options = IM_SaveOptions()) const;

private:
	float* mRGBData;
	std::shared_ptr<RenderContext> mRenderer;
};
} // namespace PR

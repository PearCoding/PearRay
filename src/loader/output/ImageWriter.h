#pragma once

#include "buffer/AOV.h"
#include "buffer/FrameBuffer.h"
#include "spectral/ToneMapper.h"

#include <vector>
#include <filesystem>

namespace PR {
struct IM_ChannelSetting1D {
	std::string Name;
	AOV1D Variable;
	int LPE = -1;
	std::string LPE_S;
};

struct IM_ChannelSettingCounter {
	std::string Name;
	AOVCounter Variable;
	int LPE = -1;
	std::string LPE_S;
};

struct IM_ChannelSetting3D {
	std::string Name[3];
	AOV3D Variable;
	int LPE = -1;
	std::string LPE_S;
};

struct IM_ChannelSettingSpec {
	std::string Name;
	ToneColorMode TCM;
	int LPE = -1;
	std::string LPE_S;
};

struct IM_SaveOptions {
	uint32 IterationMeta = 0;
	uint64 TimeMeta		 = 0; // Seconds
	bool WriteMeta		 = false;
	float SpectralFactor = 1.0f; // Used to compensate for the initial weight applied due to iteration count
};

class RenderContext;
class PR_LIB_LOADER ImageWriter {
	PR_CLASS_NON_COPYABLE(ImageWriter);

public:
	ImageWriter();
	virtual ~ImageWriter();

	void init(const std::shared_ptr<RenderContext>& renderer);
	void deinit();

	bool save(ToneMapper& toneMapper, const std::filesystem::path& file,
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

#pragma once

#include "buffer/FrameBuffer.h"
#include "buffer/OutputBuffer.h"
#include "spectral/ToneMapper.h"
#include <string>
#include <vector>

namespace PR {
struct IM_ChannelSetting1D {
	std::string Name;
	OutputBuffer::Variable1D Variable;
	ToneMapperMode TMM; //TODO: Not implemented
	int LPE = -1;
};

struct IM_ChannelSettingCounter {
	std::string Name;
	OutputBuffer::VariableCounter Variable;
	ToneMapperMode TMM; //TODO: Not implemented
	int LPE = -1;
};

struct IM_ChannelSetting3D {
	std::string Name[3];
	OutputBuffer::Variable3D Variable;
	unsigned char Elements; //TODO: Not implemented
	ToneMapperMode TMM;		//TODO: Not implemented
	int LPE = -1;
};

struct IM_ChannelSettingSpec {
	std::string Name;
	unsigned char Elements; //TODO: Not implemented
	ToneColorMode TCM;
	ToneGammaMode TGM;
	ToneMapperMode TMM;
	int LPE = -1;
};

class PR_LIB_UTILS ImageWriter {
	PR_CLASS_NON_COPYABLE(ImageWriter);

public:
	ImageWriter();
	virtual ~ImageWriter();

	void init(const std::shared_ptr<RenderContext>& renderer);
	void deinit();

	bool save(ToneMapper& toneMapper, const std::string& file,
			  const std::vector<IM_ChannelSettingSpec>& spec,
			  const std::vector<IM_ChannelSetting1D>& ch1d,
			  const std::vector<IM_ChannelSettingCounter>& chcounter,
			  const std::vector<IM_ChannelSetting3D>& ch3d) const;

	bool save_spectral(const std::string& file,
					   const std::shared_ptr<FrameBufferFloat>& spec,
					   bool compress) const;

private:
	float* mRGBData;
	std::shared_ptr<RenderContext> mRenderer;
};
} // namespace PR

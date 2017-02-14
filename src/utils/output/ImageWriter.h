#pragma once

#include "renderer/OutputChannel.h"
#include "renderer/OutputMap.h"
#include "spectral/ToneMapper.h"
#include <string>
#include <vector>

namespace PR
{
	class ToneMapper;
}

namespace PRU
{
	struct IM_ChannelSetting1D
	{
		std::string Name;
		PR::OutputMap::Variable1D Variable;
		PR::ToneMapperMode TMM;//TODO: Not implemented
	};

	struct IM_ChannelSettingCounter
	{
		std::string Name;
		PR::OutputMap::VariableCounter Variable;
		PR::ToneMapperMode TMM;//TODO: Not implemented
	};
	
	struct IM_ChannelSetting3D
	{
		std::string Name[3];
		PR::OutputMap::Variable3D Variable;
		unsigned char Elements;//TODO: Not implemented
		PR::ToneMapperMode TMM;//TODO: Not implemented
	};
	
	struct IM_ChannelSettingSpec
	{
		unsigned char Elements;//TODO: Not implemented
		PR::ToneColorMode TCM;
		PR::ToneGammaMode TGM;
		PR::ToneMapperMode TMM;
	};

	class PR_LIB_UTILS ImageWriter
	{
		PR_CLASS_NON_COPYABLE(ImageWriter);
	public:
		ImageWriter();
		virtual ~ImageWriter();

		void init(PR::RenderContext* renderer);
		void deinit();

		bool save(PR::ToneMapper& toneMapper, const std::string& file,
			IM_ChannelSettingSpec* spec,
			const std::vector<IM_ChannelSetting1D>& ch1d,
			const std::vector<IM_ChannelSettingCounter>& chcounter,
			const std::vector<IM_ChannelSetting3D>& ch3d) const;

		bool save_spectral(const std::string& file,
			PR::OutputSpectral* spec) const;

	private:
		float* mRGBData;
		PR::RenderContext* mRenderer;
	};
}
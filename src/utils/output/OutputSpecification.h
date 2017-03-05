#pragma once

#include "ImageWriter.h"
#include <list>

namespace DL
{
	class DataGroup;
}

namespace PR
{
	class ToneMapper;
	class RenderFactory;

	class Environment;
	class PR_LIB_UTILS OutputSpecification
	{
		PR_CLASS_NON_COPYABLE(OutputSpecification);
	public:
		OutputSpecification();
		virtual ~OutputSpecification();

		void init(PR::RenderFactory* factory);
		void deinit();

		void setup(PR::RenderContext* renderer);

		void parse(Environment* env, const DL::DataGroup& group);
		void save(PR::RenderContext* renderer, PR::ToneMapper& toneMapper, bool force = false) const;

	private:
		PR::RenderFactory* mRenderFactory;
		ImageWriter mImageWriter;

		struct File
		{
			std::string Name;
			IM_ChannelSettingSpec* SettingsSpectral;
			std::vector<IM_ChannelSetting1D> Settings1D;
			std::vector<IM_ChannelSettingCounter> SettingsCounter;
			std::vector<IM_ChannelSetting3D> Settings3D;
		};
		struct FileSpectral
		{
			std::string Name;
		};

		std::list<File> mFiles;
		std::list<FileSpectral> mSpectralFiles;
	};
}
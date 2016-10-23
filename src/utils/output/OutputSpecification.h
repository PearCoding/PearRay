#pragma once

#include "ImageWriter.h"
#include <list>

namespace PR
{
	class ToneMapper;
}

namespace DL
{
	class DataGroup;
}

namespace PRU
{
	class SceneLoader;
	class Environment;
	class PR_LIB_UTILS OutputSpecification
	{
		PR_CLASS_NON_COPYABLE(OutputSpecification);
	public:
		OutputSpecification();
		virtual ~OutputSpecification();

		void init(PR::Renderer* renderer);
		void deinit();
		void setup();

		void parse(SceneLoader* loader, Environment* env, DL::DataGroup* group);
		void save(PR::ToneMapper& toneMapper, bool force = false) const;

	private:
		PR::Renderer* mRenderer;
		ImageWriter mImageWriter;

		struct File
		{
			std::string Name;
			IM_ChannelSettingSpec* SettingsSpectral;
			std::vector<IM_ChannelSetting1D> Settings1D;
			std::vector<IM_ChannelSetting3D> Settings3D;
		};
		struct FileSpectral
		{
			std::string Name;
			PR::OutputSpectral* Spectral;
		};

		std::list<File> mFiles;
		std::list<FileSpectral> mSpectralFiles;
	};
}
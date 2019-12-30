#pragma once

#include "ImageWriter.h"
#include <list>

namespace DL {
class DataGroup;
}

namespace PR {
class ToneMapper;
class Environment;

class PR_LIB_UTILS OutputSpecification {
	PR_CLASS_NON_COPYABLE(OutputSpecification);

public:
	OutputSpecification(const std::wstring& wrkDir);
	virtual ~OutputSpecification();

	void init(const std::shared_ptr<RenderContext>& context);
	void deinit();
	inline bool isInit() const { return mInit; }

	void setup(const std::shared_ptr<RenderContext>& renderer);

	void parse(Environment* env, const std::vector<DL::DataGroup>& groups);
	void save(const std::shared_ptr<RenderContext>& renderer, ToneMapper& toneMapper, bool force = false) const;

private:
	bool mInit;
	std::wstring mWorkingDir;
	ImageWriter mImageWriter;

	struct File {
		std::string Name;
		std::vector<IM_ChannelSettingSpec> SettingsSpectral;
		std::vector<IM_ChannelSetting1D> Settings1D;
		std::vector<IM_ChannelSettingCounter> SettingsCounter;
		std::vector<IM_ChannelSetting3D> Settings3D;
	};
	struct FileSpectral {
		std::string Name;
		bool Compress;
		int LPE = -1;
		std::string LPE_S;
	};

	std::list<File> mFiles;
	std::list<FileSpectral> mSpectralFiles;
};
} // namespace PR

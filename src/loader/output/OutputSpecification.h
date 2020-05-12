#pragma once

#include "ImageWriter.h"
#include <vector>

namespace DL {
class DataGroup;
}

namespace PR {
class ToneMapper;
class Environment;
class FileLock;

class PR_LIB_LOADER OutputSpecification {
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
	std::unique_ptr<FileLock> mRunLock;
	std::unique_ptr<FileLock> mOutputLock;

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

	std::vector<File> mFiles;
	std::vector<FileSpectral> mSpectralFiles;
};
} // namespace PR

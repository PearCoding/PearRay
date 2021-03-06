#pragma once

#include "ImageWriter.h"

namespace DL {
class DataGroup;
}

namespace PR {
class ToneMapper;
class Environment;
class FileLock;
class FrameOutputDevice;

struct OutputSaveOptions {
	std::string NameSuffix = "";
	IM_SaveOptions Image   = IM_SaveOptions();
	bool Force			   = false;
};

class PR_LIB_LOADER OutputSpecification {
	PR_CLASS_NON_COPYABLE(OutputSpecification);

public:
	OutputSpecification(const std::filesystem::path& wrkDir);
	virtual ~OutputSpecification();

	void init(const std::shared_ptr<RenderContext>& context);
	void deinit();
	inline bool isInit() const { return mInit; }

	void setup(const std::shared_ptr<RenderContext>& context);

	void parse(Environment* env, const DL::DataGroup& group);

	/// Save whole specification by using a FrameOutputDevice
	void save(RenderContext* renderer, FrameOutputDevice* outputDevice,
			  ToneMapper& toneMapper, const OutputSaveOptions& options) const;

private:
	bool mInit;
	std::filesystem::path mWorkingDir;
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

	std::vector<File> mFiles;
};
} // namespace PR

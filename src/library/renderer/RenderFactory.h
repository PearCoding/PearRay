#pragma once

#include "RenderSettings.h"

#include <string>

namespace PR {
class RenderContext;
class Scene;
class SpectrumDescriptor;
class PR_LIB RenderFactory {
public:
	RenderFactory(const std::shared_ptr<SpectrumDescriptor>& specDesc,
				  uint32 width, uint32 height, const std::shared_ptr<Scene>& scene,
				  const std::string& workingDir);
	virtual ~RenderFactory();

	inline void setFullWidth(uint32 w) { mFullWidth = w; }
	inline uint32 fullWidth() const { return mFullWidth; }

	inline void setFullHeight(uint32 h) { mFullHeight = h; }
	inline uint32 fullHeight() const { return mFullHeight; }

	uint32 cropWidth() const;
	uint32 cropHeight() const;
	uint32 cropOffsetX() const;
	uint32 cropOffsetY() const;

	// index = image index; should be less then itx*ity!
	// itx = image tile count x
	// ity = image tile count y
	std::shared_ptr<RenderContext> create(uint32 index, uint32 itx, uint32 ity) const;
	inline std::shared_ptr<RenderContext> create() const { return create(0, 1, 1); }

	inline void setSettings(const RenderSettings& s) { mRenderSettings = s; }
	inline RenderSettings& settings() { return mRenderSettings; }
	inline const RenderSettings& settings() const { return mRenderSettings; }

	inline std::shared_ptr<Scene> scene() const { return mScene; }

	inline void setWorkingDir(const std::string& dir) { mWorkingDir = dir; }
	inline std::string workingDir() const { return mWorkingDir; }

	inline std::shared_ptr<SpectrumDescriptor> spectrumDescriptor() const { return mSpectrumDescriptor; }

private:
	uint32 mFullWidth;
	uint32 mFullHeight;
	std::string mWorkingDir;

	std::shared_ptr<Scene> mScene;

	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;

	RenderSettings mRenderSettings;
};
} // namespace PR

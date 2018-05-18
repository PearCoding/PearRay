#pragma once

#include "PR_Config.h"

#include <string>

namespace PR {
class RenderContext;
class Registry;
class Scene;
class SpectrumDescriptor;
class PR_LIB RenderFactory {
public:
	RenderFactory(const std::shared_ptr<SpectrumDescriptor>& specDesc,
				  const std::shared_ptr<Scene>& scene,
				  const std::shared_ptr<Registry>& registry,
				  const std::string& workingDir);
	virtual ~RenderFactory();

	// index = image index; should be less then itx*ity!
	// itx = image tile count x
	// ity = image tile count y
	std::shared_ptr<RenderContext> create(uint32 index, uint32 itx, uint32 ity) const;
	inline std::shared_ptr<RenderContext> create() const { return create(0, 1, 1); }

	inline std::shared_ptr<Scene> scene() const { return mScene; }
	inline std::shared_ptr<Registry> registry() const { return mRegistry; }

	inline void setWorkingDir(const std::string& dir) { mWorkingDir = dir; }
	inline std::string workingDir() const { return mWorkingDir; }

	inline std::shared_ptr<SpectrumDescriptor> spectrumDescriptor() const { return mSpectrumDescriptor; }

private:
	std::string mWorkingDir;

	std::shared_ptr<Scene> mScene;
	std::shared_ptr<Registry> mRegistry;

	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;
};
} // namespace PR

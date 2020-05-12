#pragma once

#include "RenderSettings.h"

#include <string>

namespace PR {
class IIntegrator;
class RenderContext;
class Scene;
class SpectrumDescriptor;
class PR_LIB_CORE RenderFactory {
public:
	RenderFactory(const std::shared_ptr<Scene>& scene,
				  const std::shared_ptr<SpectrumDescriptor>& specDesc);
	RenderFactory(const std::shared_ptr<Scene>& scene);
	virtual ~RenderFactory();

	// index = image index; should be less then imageTileSize.area()
	std::shared_ptr<RenderContext> create(const std::shared_ptr<IIntegrator>& integrator,
										  Point1i index, const Size2i& imageTileSize) const;
	inline std::shared_ptr<RenderContext> create(
		const std::shared_ptr<IIntegrator>& integrator) const
	{
		return create(integrator, 0, Size2i(1, 1));
	}

	inline RenderSettings& settings() { return mSettings; }
	inline const RenderSettings& settings() const { return mSettings; }

private:
	std::shared_ptr<SpectrumDescriptor> mSpectrumDescriptor;
	std::shared_ptr<Scene> mScene;
	RenderSettings mSettings;
};
} // namespace PR

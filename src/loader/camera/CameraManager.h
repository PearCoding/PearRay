#pragma once

#include "ICameraPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER CameraManager : public AbstractManager<ICameraPlugin> {
public:
	CameraManager();
	virtual ~CameraManager();

	inline void setActiveCamera(const std::shared_ptr<ICamera>& camera) { mActiveCamera = camera; }
	inline std::shared_ptr<ICamera> getActiveCamera() const { return mActiveCamera; }
	inline bool hasActiveCamera() const { return mActiveCamera != nullptr; }

private:
	std::shared_ptr<ICamera> mActiveCamera;
};
} // namespace PR

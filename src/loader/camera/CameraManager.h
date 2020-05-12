#pragma once

#include "ICameraPlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER CameraManager : public AbstractManager<ICamera, ICameraPlugin> {
public:
	CameraManager();
	virtual ~CameraManager();

	inline void setActiveCamera(uint32 id) { mActiveCamera = id; }
	inline std::shared_ptr<ICamera> getActiveCamera() const { return hasActiveCamera() ? getObject(mActiveCamera) : nullptr; }
	inline bool hasActiveCamera() const { return mActiveCamera < mObjects.size(); }

private:
	uint32 mActiveCamera;
};
} // namespace PR

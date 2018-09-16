#pragma once

#include "AbstractManager.h"

namespace PR {
class ICamera;
class ICameraFactory;

class PR_LIB CameraManager : public AbstractManager<ICamera, ICameraFactory> {
public:
	CameraManager();
	virtual ~CameraManager();

	inline void setActiveCamera(uint32 id) { mActiveCamera = id; }
	inline std::shared_ptr<ICamera> getActiveCamera() const { return getObject(mActiveCamera); }
	inline bool hasActiveCamera() const { return mActiveCamera < mObjects.size(); }

	void loadFactory(const Registry& reg,
					 const std::string& base, const std::string& name) override;

private:
	uint32 mActiveCamera;
};
} // namespace PR

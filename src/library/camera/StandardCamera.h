#pragma once

#include "Camera.h"

namespace PR {
class PR_LIB StandardCamera : public Camera {
public:
	ENTITY_CLASS

	StandardCamera(uint32 id, const std::string& name);
	virtual ~StandardCamera();

	void setWithAngle(float foh, float fov);
	void setWithSize(float width, float height);

	void setOrthographic(bool b);
	bool isOrthographic() const;

	void setWidth(float w);
	float width() const;

	void setHeight(float h);
	float height() const;

	void setLocalDirection(const Eigen::Vector3f& d);
	Eigen::Vector3f localDirection() const;

	void setLocalRight(const Eigen::Vector3f& d);
	Eigen::Vector3f localRight() const;

	void setLocalUp(const Eigen::Vector3f& d);
	Eigen::Vector3f localUp() const;

	// Depth of Field
	void setFStop(float f);
	float fstop() const;

	void setApertureRadius(float f);
	float apertureRadius() const;

	// Camera
	Ray constructRay(const CameraSample& sample) const override;

	// Entity
	std::string type() const override;
	void onFreeze() override; // Cache

private:
	void constructRay(float nx, float ny, const Eigen::Vector2f& r, Eigen::Vector3f& origin, Eigen::Vector3f& dir) const;

	bool mOrthographic;
	float mWidth;
	float mHeight;

	float mFStop;
	float mApertureRadius;

	Eigen::Vector3f mLocalDirection;
	Eigen::Vector3f mLocalRight;
	Eigen::Vector3f mLocalUp;

	// Cache:
	Eigen::Vector3f mDirection_Cache;
	Eigen::Vector3f mRight_Cache;
	Eigen::Vector3f mUp_Cache;

	bool mHasDOF_Cache;
	Eigen::Vector3f mFocalDistance_Cache;
	Eigen::Vector3f mXApertureRadius_Cache;
	Eigen::Vector3f mYApertureRadius_Cache;
};
}

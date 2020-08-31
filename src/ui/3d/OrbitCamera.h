#pragma once

#include "Camera.h"

namespace PRUI {
class PR_LIB_UI OrbitCamera : public Camera {
public:
	OrbitCamera();
	virtual ~OrbitCamera();

	inline void setLookAtPosition(const QVector3D& v) { mLookAt = v; }
	inline QVector3D lookAtPosition() const { return mLookAt; }

	virtual void pan(const QPointF& delta) override;
	virtual QMatrix4x4 getViewMatrix() const override;

private:
	QVector3D mLookAt;
};
}
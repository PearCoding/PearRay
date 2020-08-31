#pragma once

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

#include "PR_Config.h"

namespace PRUI {
class PR_LIB_UI Camera {
public:
	Camera();
	virtual ~Camera();

	inline void setPosition(const QVector3D& v) { mEye = v; }
	inline QVector3D position() const { return mEye; }

	inline void setZoom(float f) { mZoom = f; }
	inline float zoom() const { return mZoom; }

	inline void setRotation(const QQuaternion& quat) { mRotation = quat; }
	inline QQuaternion rotation() const { return mRotation; }

	virtual void pan(const QPointF& delta);

	virtual QMatrix4x4 getViewMatrix() const;

	void constructFrame(QVector3D& up, QVector3D& right);

private:
	QVector3D mEye;
	float mZoom;
	QQuaternion mRotation;
};
}
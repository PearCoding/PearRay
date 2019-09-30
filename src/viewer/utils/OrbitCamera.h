#pragma once

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class OrbitCamera {
public:
	OrbitCamera();
	virtual ~OrbitCamera();

	inline void setEyePosition(const QVector3D& v) { mEye = v; }
	inline QVector3D eyePosition() const { return mEye; }

	inline void setLookAtPosition(const QVector3D& v) { mLookAt = v; }
	inline QVector3D lookAtPosition() const { return mLookAt; }

	inline void setZoom(float f) { mZoom = f; }
	inline float zoom() const { return mZoom; }

	inline void setRotation(const QQuaternion& quat) { mRotation = quat; }
	inline QQuaternion rotation() const { return mRotation; }

	QMatrix4x4 getViewMatrix() const;

private:
	QVector3D mEye;
	QVector3D mLookAt;
	float mZoom;
	QQuaternion mRotation;
};
#include "OrbitCamera.h"

Camera::Camera()
	: mEye(0, 0, -1)
	, mZoom(1)
	, mRotation()
{
}

Camera::~Camera()
{
}

void Camera::pan(const QPointF& delta)
{
	QVector3D up, right;
	constructFrame(up, right);

	mEye += right * delta.x();
	mEye += up * delta.y();
}

QMatrix4x4 Camera::getViewMatrix() const
{
	QMatrix4x4 view;
	view.setToIdentity();
	view.translate(mEye);
	view.scale(mZoom);
	view.rotate(mRotation);

	return view;
}

void Camera::constructFrame(QVector3D& up, QVector3D& right)
{
	up	= QVector3D(0, 1, 0);
	right = QVector3D(1, 0, 0);

	up	= mRotation.rotatedVector(up);
	right = mRotation.rotatedVector(right);
}
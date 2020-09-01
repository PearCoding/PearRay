#include "OrbitCamera.h"

namespace PRUI {
Camera::Camera()
	: mEye(0, -2, 0)
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

	// Switch y&z
	view(1, 1) = 0;
	view(2, 2) = 0;
	view(1, 2) = 1;
	view(2, 1) = 1;

	view.translate(mEye);
	view.scale(mZoom);
	view.rotate(mRotation);

	return view;
}

void Camera::constructFrame(QVector3D& up, QVector3D& right)
{
	up	  = QVector3D(0, 0, 1);
	right = QVector3D(1, 0, 0);

	up	  = mRotation.rotatedVector(up);
	right = mRotation.rotatedVector(right);
}
} // namespace PRUI
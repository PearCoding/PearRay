#include "OrbitCamera.h"

OrbitCamera::OrbitCamera()
	: Camera()
	, mLookAt(0, 0, 0)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::pan(const QPointF& delta)
{
	Camera::pan(delta);

	// FIXME
	QVector3D up, right;
	constructFrame(up, right);

	mLookAt += right * delta.x();
	mLookAt += up * delta.y();
}

QMatrix4x4 OrbitCamera::getViewMatrix() const
{
	QMatrix4x4 view = Camera::getViewMatrix();
	view.translate(mLookAt);

	return view;
}

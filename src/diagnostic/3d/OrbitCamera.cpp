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
	QVector3D up, right;
	constructFrame(up, right);

	QQuaternion q = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), -delta.x() * 180)
					* QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -delta.y() * 180);

	QVector3D dir = mLookAt - position();
	dir			  = q.rotatedVector(dir);

	setPosition(mLookAt - dir);
	mLookAt = position() + dir;
}

QMatrix4x4 OrbitCamera::getViewMatrix() const
{
	QMatrix4x4 view = Camera::getViewMatrix();
	//view.translate(mLookAt);

	return view;
}

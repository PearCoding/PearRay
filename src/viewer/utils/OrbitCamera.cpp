#include "OrbitCamera.h"

OrbitCamera::OrbitCamera()
	: mEye(0, 0, -1)
	, mLookAt(0, 0, 0)
	, mZoom(1)
	, mRotation()
{
}

OrbitCamera::~OrbitCamera()
{
}

QMatrix4x4 OrbitCamera::getViewMatrix() const
{
	QMatrix4x4 view;
	view.setToIdentity();
	view.translate(mEye);
	view.scale(mZoom);
	view.rotate(mRotation);
	view.translate(mLookAt);

	return view;
}

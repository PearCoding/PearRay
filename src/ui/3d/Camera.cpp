#include "Camera.h"

namespace PR {
namespace UI {
static inline Matrix4f makeOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	float s0 = float(2) / (right - left);
	float s1 = float(2) / (top - bottom);
	float e0 = -(right + left) / (right - left);
	float e1 = -(top + bottom) / (top - bottom);

	float s2 = -float(2) / (farPlane - nearPlane);
	float e2 = -(farPlane + nearPlane) / (farPlane - nearPlane);

	Matrix4f a = Matrix4f::Zero();
	a(0, 0)	   = s0;
	a(1, 1)	   = s1;
	a(2, 2)	   = s2;
	a(3, 3)	   = 1;
	a(0, 3)	   = e0;
	a(1, 3)	   = e1;
	a(2, 3)	   = e2;
	return a;
}

static inline Matrix4f makePerspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
{
	float top	 = tan(verticalAngle / 2) * nearPlane;
	float bottom = -top;
	float right	 = top * aspectRatio;
	float left	 = -right;

	float s0 = 2 * nearPlane / (right - left);
	float s1 = 2 * nearPlane / (top - bottom);
	float e0 = (right + left) / (right - left);
	float e1 = (top + bottom) / (top - bottom);

	float s2 = -(farPlane + nearPlane) / (farPlane - nearPlane);
	float e2 = -2 * farPlane * nearPlane / (farPlane - nearPlane);

	Matrix4f a = Matrix4f::Zero();
	a(0, 0)	   = s0;
	a(1, 1)	   = s1;
	a(2, 2)	   = s2;
	a(0, 2)	   = e0;
	a(1, 2)	   = e1;
	a(2, 3)	   = e2;
	a(3, 2)	   = -1;
	return a;
}

Camera::Camera()
	: mPosition(0.0f, -1.0f, 0.0f)
	, mPivot(0.0f, 0.0f, 0.0f)
	, mOrientation()
	, mOrthogonal(false)
	, mWidth(1)
	, mHeight(1)
{
}

Camera::~Camera()
{
}

void Camera::setPosition(const Vector3f& position)
{
	mPosition = position;
}

void Camera::setOrientation(const Vector3f& right, const Vector3f& up, const Vector3f& front)
{
	setOrientation(Frame(right, up, front));
}

void Camera::setOrientation(const Frame& frame)
{
	mOrientation = frame;
}

void Camera::setPivot(const Vector3f& pivot)
{
	mPivot = pivot;
}

void Camera::setScreenSize(int width, int height)
{
	mWidth	= width;
	mHeight = height;
}

void Camera::lookAt(const Vector3f& origin, const Vector3f& pivot)
{
	Vector3f dir = pivot - origin;

	Quaternionf rot = Quaternionf::FromTwoVectors(-dir.normalized(), mOrientation.Front);
	rotate(rot);

	mPosition = origin;
	mPivot	  = pivot;
}

Matrix4f Camera::constructOrthogonalMatrix() const
{
	Matrix4f P;
	float zoom	= (mPosition - mPivot).norm() / 2.0f;
	float xzoom = zoom;
	float yzoom = zoom;
	if (width() > height())
		xzoom *= (float)width() / height();
	else
		yzoom *= (float)height() / width();

	return makeOrtho(-xzoom, xzoom, -yzoom, yzoom, 0.1f, 100000.0f);
}

Matrix4f Camera::constructPerspectiveMatrix() const
{
	float aspectRatio = width() > height() ? (float)width() / height() : (float)height() / width();
	return makePerspective(60.0f * PR::PR_DEG2RAD, aspectRatio, 0.1f, 100000.0f);
}

Matrix4f Camera::constructCameraMatrix() const
{
	if (!mOrthogonal)
		return constructPerspectiveMatrix();
	else
		return constructOrthogonalMatrix();
}

Matrix4f Camera::constructViewMatrix() const
{
	Matrix4f V = Matrix4f::Identity();
	V(0, 0)	   = mOrientation.Right.x();
	V(0, 1)	   = mOrientation.Right.y();
	V(0, 2)	   = mOrientation.Right.z();
	V(1, 0)	   = mOrientation.Up.x();
	V(1, 1)	   = mOrientation.Up.y();
	V(1, 2)	   = mOrientation.Up.z();
	V(2, 0)	   = -mOrientation.Front.x();
	V(2, 1)	   = -mOrientation.Front.y();
	V(2, 2)	   = -mOrientation.Front.z();
	V(0, 3)	   = -mOrientation.Right.dot(mPosition);
	V(1, 3)	   = -mOrientation.Up.dot(mPosition);
	V(2, 3)	   = mOrientation.Front.dot(mPosition);
	return V;
}

Matrix4f Camera::constructScreenMatrix() const
{
	return makeOrtho(0, 1, 0, 1, -1.0f, 1.0f);
}

void Camera::rotate(float delta_up, float delta_right)
{
	Quaternionf quat = Eigen::AngleAxisf(delta_up, mOrientation.Up) * Eigen::AngleAxisf(delta_right, mOrientation.Right);
	quat.normalize();
	rotate(quat);
}

void Camera::rotateView(float delta_up, float delta_right, const Vector3f& globalUp)
{
	Quaternionf quat = Eigen::AngleAxisf(delta_up, globalUp) * Eigen::AngleAxisf(delta_right, mOrientation.Right);
	quat.normalize();
	rotate(quat);
}

void Camera::zoom(float delta)
{
	float dist = std::max(0.001f, (mPosition - mPivot).norm());
	mPosition += mOrientation.Front * delta * dist;
}

void Camera::pan(float dx, float dy)
{
	mPosition -= dx * mOrientation.Right;
	mPivot -= dx * mOrientation.Right;
	mPosition += dy * mOrientation.Up;
	mPivot += dy * mOrientation.Up;
}

void Camera::translateWithPivot(const Vector3f& delta)
{
	mPosition += delta;
	mPivot += delta;
}

void Camera::rotate(const Quaternionf& rotation)
{
	if (rotation.isApprox(Quaternionf::Identity()))
		return;

	mPosition		   = rotation * (mPosition - mPivot) + mPivot;
	mOrientation.Right = (rotation * mOrientation.Right).normalized();
	mOrientation.Up	   = (rotation * mOrientation.Up).normalized();
	mOrientation.Front = (rotation * mOrientation.Front).normalized();
}

void Camera::rotateTo(const Frame& frame)
{
	Matrix3f rot = frame.matrix() * mOrientation.matrix().transpose();
	mOrientation = frame;
	mPosition	 = rot * mPosition;
}

void Camera::makeFrontView(const Frame& oF)
{
	rotateTo(oF);
}

void Camera::makeBackView(const Frame& oF)
{
	Frame nF;
	nF.Front = -oF.Front;
	nF.Up	 = oF.Up;
	nF.Right = -oF.Right;
	rotateTo(nF);
}

void Camera::makeTopView(const Frame& oF)
{
	Frame nF;
	nF.Front = -oF.Up;
	nF.Up	 = oF.Front;
	nF.Right = oF.Right;
	rotateTo(nF);
}

void Camera::makeBottomView(const Frame& oF)
{
	Frame nF;
	nF.Front = oF.Up;
	nF.Up	 = -oF.Front;
	nF.Right = oF.Right;
	rotateTo(nF);
}

void Camera::makeRightView(const Frame& oF)
{
	Frame nF;
	nF.Front = -oF.Right;
	nF.Up	 = oF.Up;
	nF.Right = oF.Front;
	rotateTo(nF);
}

void Camera::makeLeftView(const Frame& oF)
{
	Frame nF;
	nF.Front = oF.Right;
	nF.Up	 = oF.Up;
	nF.Right = -oF.Front;
	rotateTo(nF);
}

void Camera::makeOrthogonal(bool b)
{
	mOrthogonal = b;
}

Vector3f Camera::projectPoint(const Vector2f& pos) const
{
	Matrix4f invProj = (constructCameraMatrix() * constructViewMatrix()).inverse();
	const Vector4f p = invProj * Vector4f(-1 + 2 * pos.x() / width(), 1 - 2 * pos.y() / height(), -1.0f, 1);
	return p.block<3, 1>(0, 0) / p(3);
}

Vector2f Camera::projectPoint(const Vector3f& pos) const
{
	Matrix4f proj	 = (constructCameraMatrix() * constructViewMatrix());
	const Vector4f p = proj * Vector4f(pos(0), pos(1), pos(2), 1);
	return Vector2f((p.x() / p.w() + 1) * 0.5f * width(), (1 - (p.y() / p.w() + 1) * 0.5f) * height()) / (p.z() / p.w());
}

Vector3f Camera::projectLine(const Vector2f& line) const
{
	Matrix4f invProj = (constructCameraMatrix() * constructViewMatrix()).inverse();
	return invProj.block<3, 3>(0, 0) * Vector3f(2 * line.x() / width(), -2 * line.y() / height(), 0.0f);
}

float Camera::projectWidth(float w) const { return projectLine(Vector2f(w, 0)).norm(); }
float Camera::projectHeight(float h) const { return projectLine(Vector2f(0, h)).norm(); }

void Camera::constructRay(const Vector2f& screen_point, Vector3f& origin, Vector3f& direction) const
{
	origin = projectPoint(screen_point);
	if (mOrthogonal)
		direction = -mOrientation.Front.normalized();
	else
		direction = (origin - mPosition).normalized();
}
} // namespace UI
} // namespace PR
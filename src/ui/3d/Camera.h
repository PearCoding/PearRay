#pragma once

#include "PR_Config.h"

namespace PR {
namespace UI {
/// 3D orthogonal frame/base
struct PR_LIB_UI Frame {
	Vector3f Front = Vector3f(0, 0, 1);
	Vector3f Up	   = Vector3f(0, 1, 0);
	Vector3f Right = Vector3f(1, 0, 0);

	inline Frame() = default;
	inline Frame(const Vector3f& right, const Vector3f& up, const Vector3f& front)
		: Front(front)
		, Up(up)
		, Right(right)
	{
	}

	inline Matrix3f matrix() const
	{
		Matrix3f mat;
		mat(0, 0) = Right(0);
		mat(1, 0) = Right(1);
		mat(2, 0) = Right(2);
		mat(0, 1) = Up(0);
		mat(1, 1) = Up(1);
		mat(2, 1) = Up(2);
		mat(0, 2) = Front(0);
		mat(1, 2) = Front(1);
		mat(2, 2) = Front(2);
		return mat;
	}
};

/// Standard 3D camera based on frame around pivot
class PR_LIB_UI Camera {
public:
	Camera();
	virtual ~Camera();

	/// Set position of camera
	void setPosition(const Vector3f& position);
	/// Set orientation of camera
	void setOrientation(const Vector3f& right, const Vector3f& up, const Vector3f& front);
	/// Set orientation of camera
	void setOrientation(const Frame& frame);
	/// Set pivot of camera
	void setPivot(const Vector3f& pivot);
	/// Set screen size of camera
	void setScreenSize(int width, int height);

	/// Reposition to origin and orientate camera such that the new pivot is visible
	void lookAt(const Vector3f& origin, const Vector3f& pivot);
	/// Orientate camera such that the new pivot is visible
	inline void lookAt(const Vector3f& pivot) { lookAt(mPosition, pivot); }
	/// Ensure that the pivot is visible
	inline void ensureVisiblePivot() { lookAt(mPosition, mPivot); }

	inline const Vector3f& position() const { return mPosition; }
	inline const Vector3f& pivot() const { return mPivot; }
	inline const Vector3f& frontVector() const { return mOrientation.Front; }
	inline const Vector3f& upVector() const { return mOrientation.Up; }
	inline const Vector3f& rightVector() const { return mOrientation.Right; }
	inline const Frame& orientation() const { return mOrientation; }

	inline int width() const { return mWidth; }
	inline int height() const { return mHeight; }

	/// Rotate around current up axis and current right axis
	/// \param delta_up Angle around current up axis in degrees
	/// \param delta_right Angle around current right axis in degrees
	void rotate(float delta_up, float delta_right);
	/// Rotate around global up axis and current right axis
	/// \details This function tries to simulate the behaviour of the 3d modeling software Blender
	/// \param delta_up Angle around (0,0,1) in degrees
	/// \param delta_right Angle around current right axis in degrees
	/// \param globalUp Global up axis used instead of current up axis
	void rotateView(float delta_up, float delta_right, const Vector3f& globalUp = Vector3f(0, 0, 1));
	/// Zoom towards pivot point with the front axis
	/// \param delta Factor measured in distance of pivot and current position
	void zoom(float delta);
	/// View panning
	/// \param dx Translation based on the right axis
	/// \param dy Translation based on the up axis
	void pan(float dx, float dy);
	/// Translation of current position and pivot
	void translateWithPivot(const Vector3f& delta);

	/// Rotation of camera frame and position
	void rotate(const Quaternionf& quat);
	/// Rotation of camera frame towards frame by also changing current position
	void rotateTo(const Frame& frame);

	/// Approximative zoom factor
	inline float approximativeZoomFactor() const { return (mPivot - mPosition).norm(); }

	/// Projection matrix depending if orthogonal or not
	Matrix4f constructCameraMatrix() const;
	/// Inverse object matrix of the camera
	Matrix4f constructViewMatrix() const;
	/// Orthogonal matrix used for 2d projection
	Matrix4f constructScreenMatrix() const;

	/// Orthogonal projection matrix
	Matrix4f constructOrthogonalMatrix() const;
	/// Perspective projection matrix
	Matrix4f constructPerspectiveMatrix() const;

	/// Projecting 2d screen point to 3d point on the viewing plane
	Vector3f projectPoint(const Vector2f& pos) const;
	/// Projecting 3d global point to 2d screen point
	Vector2f projectPoint(const Vector3f& pos) const;

	/// Projecting 2d line to a 3d line on the viewing plane
	Vector3f projectLine(const Vector2f& line) const;
	/// Projecting width in the screen space to a distance in global space
	float projectWidth(float w) const;
	/// Projecting height in the screen space to a distance in global space
	float projectHeight(float h) const;

	/// Constructing ray based on current projection
	void constructRay(const Vector2f& pos, Vector3f& origin, Vector3f& direction) const;

	/// Switch to frame viewing frontwise
	void makeFrontView(const Frame& originalFrame = Frame());
	/// Switch to frame viewing backwise
	void makeBackView(const Frame& originalFrame = Frame());
	/// Switch to frame viewing topwise
	void makeTopView(const Frame& originalFrame = Frame());
	/// Switch to frame viewing bottomwise
	void makeBottomView(const Frame& originalFrame = Frame());
	/// Switch to frame viewing rightwise
	void makeRightView(const Frame& originalFrame = Frame());
	/// Switch to frame viewing leftwise
	void makeLeftView(const Frame& originalFrame = Frame());

	/// Change projection to orthogonal
	void makeOrthogonal(bool b);

	/// Returns if current projection is orthogonal
	inline bool isOrthogonal() const { return mOrthogonal; }

private:
	Vector3f mPosition;
	Vector3f mPivot;
	Frame mOrientation;
	bool mOrthogonal;
	int mWidth;
	int mHeight;
};
} // namespace UI
} // namespace PR
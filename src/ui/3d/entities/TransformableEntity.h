#pragma once

#include "PR_Config.h"

namespace PR {
namespace UI {
// The Transform convention is TRS!
class PR_LIB_UI TransformableEntity {
public:
	TransformableEntity();
	virtual ~TransformableEntity() = default;

	inline const Matrix4f& transform() const { return mTransformCache; }
	// TRS
	void setTransform(const Matrix4f& trans);
	void setTransform(const Vector3f& pos, const Quaternionf& rot, const Vector3f& scale);
	void setTransform(const Vector3f& pos, const Quaternionf& rot, float scale);
	void setTransform(const Vector3f& pos, const Quaternionf& rot);
	// TR
	void setTransformNoScale(const Matrix4f& trans);
	Matrix4f transformNoScale() const;

	Matrix3f normalTransform() const;

	inline const Vector3f& position() const { return mTranslation; }
	inline const Vector3f& translation() const { return mTranslation; }
	inline const Quaternionf& rotation() const { return mRotation; }
	inline const Vector3f& scale() const { return mScale; }

	// Operators do not interfere with eachother -> Translation is applied before rotation...
	void translate(const Vector3f& v);
	void rotate(const Quaternionf& v);
	void scale(const Vector3f& v);
	void scale(float f);

	void setTranslation(const Vector3f& v);
	void setRotation(const Quaternionf& v);
	void setScale(const Vector3f& v);
	void setScale(float f);

	inline void show(bool b = true) { mVisible = b; }
	inline void hide(bool b = true) { show(!b); }
	inline bool isVisible() const { return mVisible; }
	inline bool isHidden() const { return !mVisible; }

private:
	void cache();
	Matrix4f calcMatrix() const;

	Vector3f mTranslation;
	Quaternionf mRotation;
	Vector3f mScale;
	Matrix4f mTransformCache;

	bool mVisible;
};
} // namespace UI
} // namespace PR
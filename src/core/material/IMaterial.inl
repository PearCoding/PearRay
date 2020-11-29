// IWYU pragma: private, include "material/IMaterial.h"
namespace PR {

inline bool IMaterial::canBeShaded() const { return mCanBeShaded; }
inline void IMaterial::enableShading(bool b) { mCanBeShaded = b; }
inline void IMaterial::enableShadow(bool b) { mShadow = b; }
inline bool IMaterial::allowsShadow() const { return mShadow; }
inline void IMaterial::enableSelfShadow(bool b) { mSelfShadow = b; }
inline bool IMaterial::allowsSelfShadow() const { return mSelfShadow; }
inline void IMaterial::enableCameraVisibility(bool b) { mCameraVisible = b; }
inline bool IMaterial::isCameraVisible() const { return mCameraVisible; }

} // namespace PR
